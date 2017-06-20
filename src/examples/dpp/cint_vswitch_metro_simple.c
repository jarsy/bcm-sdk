 /* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *  Diagram:                    
 *  |               -----------                        |
 *  |              /  \   /\   \                       |
 *  |        CP1  |    \  /     |   SP1        		   |
 *  |      Y C20  |     \/      |   S300,C21 Y         |
 *  |      R C30  |     /\      |   S300,C30 R         |
 *  |        CP2  |    /  \     |   SP2                |             
 *  |      G C10  |   \/   \    |   S300,C21 Y         |
 *  |              \  VSwitch  /    S300,C10 G         | 
 *  |               -----------             		   |
 *  Simple VSWITCH example, where we have 4 ports (SP1,SP2, CP1,CP2) 
 *  3 flooding groups (R,G,Y) and 7 Attachment Circuts (C20, C30,...)
 *
 * Test Scenario 
 *
 * ./bcm.user
 * cd ../../
 * cint ../../src/examples/dpp/utility/cint_utils_global.c
 * cint ../../src/examples/dpp/utility/cint_utils_multicast.c
 * cint ../../src/examples/dpp/utility/cint_utils_vlan.c
 * cint ../../src/examples/dpp/cint_vswitch_metro_simple.c
 * cint
 * vswitch_metro_simple_run(0,200,201,202,203);
 * exit;
 *
 * tx 1 psrc=200 data=0x000000000011000074ddb68f8100012c91000015ffff
 *
 * /* Received packets on unit 0 should be: */
 * /* 0x000000000011000074ddb68f81000014ffff0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 */
 * /* 0x000000000011000074ddb68f8100012c91000015ffff0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 */
 *
 */
 
struct vswitch_metro_simple_s {
    int vswitch_start; 
    int vlan_ports[7];
    int vswitchs[3];
    int multicasts[3];
    bcm_vlan_action_set_t action;
    int multicast_vswitch;
};

/*                 vswitch_start | vlan_ports |       */
vswitch_metro_simple_s g_vswitch_metro_simple = {4096,          {0}, 
/*                 vswitchs  | multicasts */
                   {0},           {0},
/*                 action  | multicast_vswitch */
                   {0},           0
};



/* Create a C-port that recognize C-VLANs only */
int vswitch_metro_simple_create_c_port(int unit, int port) 
{
  int rv;

  /* Port TPIDs */
  rv = port_tpid_init(port,1,0);
  if (rv != BCM_E_NONE) {
      printf("Error, port_tpid_init\n");
      return rv;
  }
  rv = port_tpid_set(unit);
  if (rv != BCM_E_NONE) {
      printf("Error, port_tpid_set\n");
      return rv;
  }
  /* VLAN Domains */
  rv = bcm_port_class_set(unit, port, bcmPortClassId, port);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_class_set\n");
      return rv;
  }
  /* Same interface */
  rv = bcm_port_control_set(unit,port,bcmPortControlBridge,1);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_control_set\n");
      return rv;
  }
  return rv;
}

/* Create a S-port that recognize S-VLAN, C-VLAN onlys*/
int vswitch_metro_simple_create_s_port(int unit, int port) 
{
  int rv;

  /* Port TPIDs */
  rv = port_tpid_init(port,1,1);
  if (rv != BCM_E_NONE) {
      printf("Error, port_tpid_init\n");
      return rv;
  }
  rv = port_tpid_set(unit);
  if (rv != BCM_E_NONE) {
      printf("Error, port_tpid_set\n");
      return rv;
  }
  /* VLAN Domains */
  rv = bcm_port_class_set(unit, port, bcmPortClassId, port);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_class_set\n");
      return rv;
  }
  /* Same interface */
  rv = bcm_port_control_set(unit,port,bcmPortControlBridge,1);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_control_set\n");
      return rv;
  }

  return rv;
}

/* Global Configuration */
int vswitch_metro_simple_global(int unit) {
  bcm_vlan_action_set_t_init(&g_vswitch_metro_simple.action);
  g_vswitch_metro_simple.action.dt_outer = bcmVlanActionDelete;
  g_vswitch_metro_simple.action.dt_inner = bcmVlanActionDelete;
  g_vswitch_metro_simple.action.ot_outer = bcmVlanActionDelete;

  return 0;
}

/* Main function to run */
int vswitch_metro_simple_run(int unit, int s_port1, int s_port2, int c_port1, int c_port2) {

  int i;
  int rv;
  int is_vlan_translate_mode = 0;

  is_vlan_translate_mode = soc_property_get(unit , "bcm886xx_vlan_translate_mode",0);
  

  if (is_vlan_translate_mode == 0) {
      /* Global */
      rv = vswitch_metro_simple_global(unit);
      if (rv != BCM_E_NONE) {
        printf("Error, global\n");
        return rv;
      }

      /* Port */
      rv = vswitch_metro_simple_create_c_port(unit, c_port1);
      if (rv != BCM_E_NONE) {
        printf("Error, create_c_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_c_port(unit, c_port2);
      if (rv != BCM_E_NONE) {
        printf("Error, create_c_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_port(unit, s_port1);
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_port(unit, s_port2);
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_port\n");
        return rv;
      }

      /* Vswitch */
      for (i = 0; i < 3; i++)
      {
        rv = vswitch_metro_simple_create_vswitch(unit, &(g_vswitch_metro_simple.vswitchs[i]),&(g_vswitch_metro_simple.multicasts[i]));
        if (rv != BCM_E_NONE) {
            printf("Error, create_vswitch\n");
            return rv;
        }
      }

      /* Create VLAN-Ports */
      rv = vswitch_metro_simple_create_s_c_vlan_port(unit, s_port1, 300, 21, &(g_vswitch_metro_simple.vlan_ports[0]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_c_vlan_port(unit, s_port1, 300, 30, &(g_vswitch_metro_simple.vlan_ports[1]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_c_vlan_port(unit, s_port2, 300, 21, &(g_vswitch_metro_simple.vlan_ports[2]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_c_vlan_port(unit, s_port2, 300, 10, &(g_vswitch_metro_simple.vlan_ports[3]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_c_vlan_port(unit, c_port1, 20, &(g_vswitch_metro_simple.vlan_ports[4]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_c_vlan_port(unit, c_port1, 30, &(g_vswitch_metro_simple.vlan_ports[5]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_c_vlan_port(unit, c_port2, 10, &(g_vswitch_metro_simple.vlan_ports[6]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      /* Associate VLAN-port to VSWITCH and add to flooding group */
      /* Yellow */
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[0], g_vswitch_metro_simple.multicasts[0], s_port1, g_vswitch_metro_simple.vlan_ports[0]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[0], g_vswitch_metro_simple.multicasts[0], s_port2, g_vswitch_metro_simple.vlan_ports[2]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[0], g_vswitch_metro_simple.multicasts[0], c_port1, g_vswitch_metro_simple.vlan_ports[4]);
      if (rv != BCM_E_NONE) {
          printf("Error, associate_vlan_port_to_vsi\n");
          return rv;
      }
      /* Red */
      rv =  vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[1], g_vswitch_metro_simple.multicasts[1], s_port1, g_vswitch_metro_simple.vlan_ports[1]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[1], g_vswitch_metro_simple.multicasts[1], c_port1, g_vswitch_metro_simple.vlan_ports[5]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      /* Green */
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[2], g_vswitch_metro_simple.multicasts[2], s_port2, g_vswitch_metro_simple.vlan_ports[3]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[2], g_vswitch_metro_simple.multicasts[2], c_port2, g_vswitch_metro_simple.vlan_ports[6]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
  }

  return rv;
}

/* Main function to run in AVT mode*/
int vswitch_metro_avt_run(int unit, int s_port1, int s_port2, int c_port1, int c_port2) {

  int i;
  int rv;
  int is_vlan_translate_mode = 0;

  is_vlan_translate_mode = soc_property_get(unit , "bcm886xx_vlan_translate_mode",0);
  

  if (is_vlan_translate_mode) {
      /* only for Regression: AT_Cint_vswitch_metro_simple */
      /* Y */
      int action_flags = BCM_VLAN_ACTION_SET_WITH_ID;
      bcm_vlan_action_set_t action;
      bcm_vlan_translate_action_class_t action_class;
      int action_id_2 = 6;
      int action_id_3 = 7;
      uint32 edit_profile_2 =3;
      uint32 edit_profile_3 =4;

      /* Port */
      rv = vswitch_metro_simple_create_s_port(unit, c_port1);
      if (rv != BCM_E_NONE) {
        printf("Error, create_c_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_port(unit, s_port1);
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_port\n");
        return rv;
      }
      rv = vswitch_metro_simple_create_s_port(unit, s_port2);
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_port\n");
        return rv;
      }

      /* Create VLAN-Ports */
      rv = vswitch_metro_avt_create_s_c_vlan_port(unit, s_port1, 300, 21, &(g_vswitch_metro_simple.vlan_ports[0]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      /* set NOP for ingress LIF */
      rv = vlan_port_translation_set(unit, 300, 21, g_vswitch_metro_simple.vlan_ports[0], 0, 1);
      if (rv != BCM_E_NONE) {
          printf("Error, vlan_port_translation_set\n");
          return rv;
      }

      rv = vswitch_metro_avt_create_s_c_vlan_port(unit, s_port2, 300, 21, &(g_vswitch_metro_simple.vlan_ports[2]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }
      /* set edit profile for egress LIF */
      rv = vlan_port_translation_set(unit, 300, 21, g_vswitch_metro_simple.vlan_ports[2], edit_profile_2, 0);
      if (rv != BCM_E_NONE) {
          printf("Error, vlan_port_translation_set\n");
          return rv;
      }

      rv = vswitch_metro_avt_create_s_c_vlan_port(unit, c_port1, 300, 21, &(g_vswitch_metro_simple.vlan_ports[4]));
      if (rv != BCM_E_NONE) {
        printf("Error, create_s_c_vlan_port\n");
        return rv;
      }

      /* set edit profile for egress LIF */
      rv = vlan_port_translation_set(unit, 20, 0, g_vswitch_metro_simple.vlan_ports[4], edit_profile_3, 0);
      if (rv != BCM_E_NONE) {
          printf("Error, vlan_port_translation_set\n");
          return rv;
      }

      /* Vswitch:Y */
      rv = vswitch_metro_simple_create_vswitch(unit, &(g_vswitch_metro_simple.vswitchs[0]),&(g_vswitch_metro_simple.multicasts[0]));

      rv = bcm_vlan_translate_action_id_create(unit, action_flags|BCM_VLAN_ACTION_SET_EGRESS, &action_id_2);
      if (rv != BCM_E_NONE)
      {
          printf("Error, bcm_vlan_translate_action_id_create\n");
          return rv;
      }

      bcm_vlan_action_set_t_init(&action);
      action.dt_outer = bcmVlanActionNone;
      action.dt_inner = bcmVlanActionNone;
      action.outer_tpid = 0x8100;
      action.inner_tpid = 0x9100;

      rv = bcm_vlan_translate_action_id_set( unit, BCM_VLAN_ACTION_SET_EGRESS, action_id_2, &action);
      if (rv != BCM_E_NONE)
      {
          printf("Error, bcm_vlan_translate_action_id_set\n");
          return rv;
      }

      bcm_vlan_translate_action_class_t_init(&action_class);
      action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
      action_class.vlan_edit_class_id = edit_profile_2;
      action_class.vlan_translation_action_id = action_id_2;
      action_class.tag_format_class_id = 6; /* Format S-C */
      rv = bcm_vlan_translate_action_class_set(unit, &action_class);
      if (rv != BCM_E_NONE)
      {
          printf("Error, bcm_vlan_translate_action_class_set\n");
          return rv;
      }

      rv = bcm_vlan_translate_action_id_create(unit, action_flags|BCM_VLAN_ACTION_SET_EGRESS, &action_id_3);
      if (rv != BCM_E_NONE)
      {
          printf("Error, bcm_vlan_translate_action_id_create\n");
          return rv;
      }

      bcm_vlan_action_set_t_init(&action);
      action.dt_outer = bcmVlanActionDelete;
      action.dt_inner = bcmVlanActionReplace;
      action.outer_tpid = 0x8100;
      action.inner_tpid = 0x9100;

      rv = bcm_vlan_translate_action_id_set( unit, BCM_VLAN_ACTION_SET_EGRESS, action_id_3, &action);
      if (rv != BCM_E_NONE)
      {
          printf("Error, bcm_vlan_translate_action_id_set\n");
          return rv;
      }

      bcm_vlan_translate_action_class_t_init(&action_class);
      action_class.flags = BCM_VLAN_ACTION_SET_EGRESS;
      action_class.vlan_edit_class_id = edit_profile_3;
      action_class.vlan_translation_action_id = action_id_3;
      action_class.tag_format_class_id = 6; /* Format S-C */
      rv = bcm_vlan_translate_action_class_set(unit, &action_class);
      if (rv != BCM_E_NONE)
      {
          printf("Error, bcm_vlan_translate_action_class_set\n");
          return rv;
      }
      /* Yellow */
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[0], g_vswitch_metro_simple.multicasts[0], s_port1, g_vswitch_metro_simple.vlan_ports[0]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[0], g_vswitch_metro_simple.multicasts[0], s_port2, g_vswitch_metro_simple.vlan_ports[2]);
      if (rv != BCM_E_NONE) {
        printf("Error, associate_vlan_port_to_vsi\n");
        return rv;
      }
      rv = vswitch_metro_simple_associate_vlan_port_to_vsi(unit, g_vswitch_metro_simple.vswitchs[0], g_vswitch_metro_simple.multicasts[0], c_port1, g_vswitch_metro_simple.vlan_ports[4]);
      if (rv != BCM_E_NONE) {
          printf("Error, associate_vlan_port_to_vsi\n");
          return rv;
      }
  }

  return rv;
}

/* Create VSWITCH */
int vswitch_metro_simple_create_vswitch(int unit, int* vswitch, int* multicast_id) 
{
  int rv;
  g_vswitch_metro_simple.multicast_vswitch = g_vswitch_metro_simple.vswitch_start;
  /* Create MC-ID */
  rv = multicast__open_mc_group(unit,&g_vswitch_metro_simple.multicast_vswitch,0);
  if (rv != BCM_E_NONE) {
      printf("Error, multicast__open_mc_group\n");
      return rv;
  }
  /* Create VSI */
  rv = vlan__open_vlan_per_mc(unit,g_vswitch_metro_simple.vswitch_start,g_vswitch_metro_simple.multicast_vswitch);
  if (rv != BCM_E_NONE) {
      printf("Error, vlan__open_vlan_per_mc\n");
      return rv;
  }
  *vswitch = g_vswitch_metro_simple.vswitch_start;
  *multicast_id = g_vswitch_metro_simple.multicast_vswitch;
  g_vswitch_metro_simple.vswitch_start++;

  return rv;
}

/* Create VLAN port that recognize S-C VLANs , working in untagged canonical format */
int vswitch_metro_simple_create_s_c_vlan_port(int unit, int port, int s_vlan, int c_vlan, int* vlan_port_id) 
{
  int rv;
  bcm_vlan_port_t vlan_port;
  bcm_vlan_port_t_init(&vlan_port);
  /* Create S-VLANs x C-VLANs */
  vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;    
  vlan_port.port = port;
  vlan_port.match_vlan = s_vlan;
  vlan_port.match_inner_vlan = c_vlan;
  vlan_port.egress_vlan = s_vlan;
  vlan_port.egress_inner_vlan = c_vlan;
  rv = bcm_vlan_port_create(unit, &vlan_port);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_port_create\n");
      return rv;
  }
  *vlan_port_id = vlan_port.vlan_port_id;
  /* IVE */
  rv = bcm_vlan_translate_action_create(unit, *vlan_port_id, bcmVlanTranslateKeyPortOuter, BCM_VLAN_INVALID, BCM_VLAN_INVALID, &g_vswitch_metro_simple.action);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_translate_action_create\n");
      return rv;
  }

  return rv;
}

/* Create VLAN-Port C-VLAN */
int vswitch_metro_simple_create_c_vlan_port(int unit, int port, int c_vlan, int* vlan_port_id)
{
  int rv;
  bcm_vlan_port_t vlan_port;
  bcm_vlan_port_t_init(&vlan_port);
  /* Create S-VLANs x C-VLANs */
  vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;    
  vlan_port.port = port;
  vlan_port.match_vlan = c_vlan;
  vlan_port.egress_vlan = c_vlan;
  rv = bcm_vlan_port_create(unit, &vlan_port);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_port_create\n");
      return rv;
  }
  *vlan_port_id = vlan_port.vlan_port_id;
  /* IVE */
  rv = bcm_vlan_translate_action_create(unit, *vlan_port_id, bcmVlanTranslateKeyPortOuter, BCM_VLAN_INVALID, BCM_VLAN_INVALID, &g_vswitch_metro_simple.action);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_translate_action_create\n");
      return rv;
  }

  return rv;
}


int vswitch_metro_avt_create_s_c_vlan_port(int unit, int port, int s_vlan, int c_vlan, int* vlan_port_id)
{
  int rv;
  bcm_vlan_port_t vlan_port;

  bcm_vlan_port_t_init(&vlan_port);
  /* Create S-VLANs x C-VLANs */
  vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN_STACKED;
  vlan_port.port = port;
  vlan_port.match_vlan = s_vlan;
  vlan_port.match_inner_vlan = c_vlan;
  vlan_port.egress_vlan = s_vlan;
  vlan_port.egress_inner_vlan = c_vlan;
  rv = bcm_vlan_port_create(unit, &vlan_port);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_port_create\n");
      return rv;
  }
  *vlan_port_id = vlan_port.vlan_port_id;

  return rv;
}

/* Associate VLAN-Port to VSWITCH */
int vswitch_metro_simple_associate_vlan_port_to_vsi(int unit, int vswitch, int multicast_id, int port, int vlan_port_id) 
{
  int rv;
  rv = bcm_vswitch_port_add(unit, vswitch,vlan_port_id);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vswitch_port_add\n");
      return rv;
  }
  rv = multicast__vlan_encap_add(unit,multicast_id,port,vlan_port_id);
  if (rv != BCM_E_NONE) {
      printf("Error, multicast__vlan_encap_add\n");
      return rv;
  }
  return rv;
}

