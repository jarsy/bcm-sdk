/* $Id: cint_vlan_port_initial_vid.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/*
 * File: cint_vlan_port_initial_vid.c
 * Purpose: Examples of initial-VID.
 *          - By default, bcm API differs between tagged packets and untagged packets by Initial-VID database. 
 *          - CINT introduce per port decision if to act as default or always to use Initial-VID with no difference
 *            between untagged packets and tagged packets. Database used in this case are match MATCH_PORT_VLAN and
 *            MATCH_PORT_VLAN_VLAN (no use for MATCH_PORT_INITIAL_VLAN).
 *
 * Calling sequence:
 *
 * Initialization:
 *   1. Add the following port configureations to config-sand.bcm, so use Initial-VID with no difference
 *      between untagged packets and tagged packets.
 *      vlan_translation_initial_vlan_enable_<port>=0
 *
 *   2. Set differnce vlan domain per port. 
 *      - call examples_initial_vid_init(unit, port1, port2, port3);
 *
 *   3. Set VT profile as double tag priority initial vid.
 *      - call examples_initial_vid_set(unit, in_port, enable);
 *
 *   4. create a pp model with double tag AC
 *      - vswitch_metro_mp_run_with_defaults(unit, port1, port2, port3);
 *
 *   5. create a single tag AC, and connect with vsi.
 *      - examples_initial_vid_setup_lif(unit, in_port, vlan, vsi, 0);
 *
 *   6. create a initial vid AC for untag packet, and connect with vsi.
 *      - examples_initial_vid_setup_lif(unit, in_port, vlan, vsi, 1);
 *
 *   7. send double tag(10, 20), single tag(10), untag packet, can receive it in port 16.
 *      Port 15 outer vid 10 inner vid 20 ---> vsi ---> Port 16 outer vid 3 inner vid 6
 *              outer vid 10              <---  | <---       outer vid 5 inner vid 2
 *              untag                     <---  | <---       outer vid 5 inner vid 2
 *              outer vid 30 inner vid 60 <---  | <---       outer vid 5 inner vid 2
 *        - From port 15:
 *              - double tag
 *                - ethernet header with any DA, SA 
 *                - outer vid 10
 *                - inner vid 20
 *              - single tag
 *                - ethernet header with any DA, SA 
 *                - outer vid 10
 *              - untag
 *                - ethernet header with any DA, SA
 *
 *        - From port 16:
 *              -   ethernet header with any DA, SA
 *              -   outer vid 5
 *              -   inner vid 2
 *
 * To Activate Above Settings:
 *      BCM> cint src/examples/dpp/cint_port_tpid.c
 *      BCM> cint src/examples/dpp/cint_vlan_port_initial_vid.c
 *      BCM> cint src/examples/dpp/cint_l2_mact.c
 *      BCM> cint src/examples/dpp/cint_vswitch_metro_mp.c
 *      BCM> cint src/examples/dpp/cint_multi_device_utils.c
 *      BCM> cint
 *      cint> 
 *      cint> print examples_initial_vid_init(unit, port1, port2, port3);
 *      cint> print examples_initial_vid_set(unit, in_port, enable);
 *      cint> print vswitch_metro_mp_run_with_defaults(unit, port1, port2, port3);
 *      cint> print examples_initial_vid_setup_lif(unit, in_port, vlan, vsi, 0);
 *      cint> print examples_double_lookup_initial_vid_set(unit, in_port, vlan, vsi, 1);
 *
 * 
 * Notes: Please see cint_vswitch_metro_mp.c to know pp model.
 * 
 */

/* Set differnce vlan domain per port */
int examples_initial_vid_init(int unit, int init_vid_port, int port2, int port3)
{
  int rv  = 0;

  rv = bcm_port_class_set(unit, init_vid_port, bcmPortClassId, init_vid_port);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_class_set unit %d, port %d rv %d\n", unit, init_vid_port, rv);
      return rv;
  }

  rv = bcm_port_class_set(unit, port2, bcmPortClassId, port2);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_class_set unit %d, port %d rv %d\n", unit, port2, rv);
      return rv;
  }

  rv = bcm_port_class_set(unit, port3, bcmPortClassId, port3);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_port_class_set unit %d, port %d rv %d\n", unit, port3, rv);
      return rv;
  }

  return rv;

}

/* Create a single tag AC, or a initial-vid AC for untag packet */
int examples_initial_vid_setup_lif(int unit, int in_port, int vlan, int vsi, int is_init_vid_ac)
{
  bcm_vlan_port_t pvid_map;
  int rv  = 0;

  if (is_init_vid_ac)
  {
    rv =  bcm_port_untagged_vlan_set(unit, in_port, vlan);
    if (rv != BCM_E_NONE) {
        printf("Error, bcm_port_untagged_vlan_set unit %d, in_port %d, vid %d, rv %d\n", unit, in_port, vlan, rv);
        return rv;
    }
  }

  bcm_vlan_port_t_init(&pvid_map);
  if (is_init_vid_ac)
  {
    pvid_map.criteria = BCM_VLAN_PORT_MATCH_PORT_INITIAL_VLAN;
  }
  else
  {
    pvid_map.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
  }
  pvid_map.match_vlan = vlan;
  pvid_map.vsi = 0;
  pvid_map.port = in_port;
  rv = bcm_vlan_port_create(unit, &pvid_map);
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_port_create unit %d, in_port %d, vid %d, vsi:%d rv %d\n", unit, in_port, vlan, vsi, rv);
      return rv;
  }

  rv = vswitch_add_port(unit, vsi,in_port, pvid_map.vlan_port_id);
  if (rv != BCM_E_NONE) {
      printf("Error, vswitch_add_port\n");
      return rv;
  }

  return rv;
}

/* Set VT profile as double tag priority initial vid */
int examples_double_lookup_initial_vid_set(int unit, int init_vid_port, int enable)
{
  int rv  = 0;

  rv = bcm_vlan_control_port_set(unit, init_vid_port, bcmVlanPortDoubleLookupEnable, enable); 
  if (rv != BCM_E_NONE) {
      printf("Error, bcm_vlan_control_port_set unit %d, port %d rv %d\n", unit, init_vid_port, rv);
      return rv;
  }

  return rv;
}


