/*
 * $Id: stg.h,v 1.1.2.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains STG definitions internal to the BCM library.
 */

#ifndef _BCM_INT_SBX_STG_H_
#define _BCM_INT_SBX_STG_H_

#define BCM_STG_MAX     BCM_VLAN_COUNT

/*
 * Function:
 *      _bcm_fe2000_stg_map_get
 * Purpose:
 *      Get STG that a VLAN is mapped to.
 * Parameters:
 *      unit - device unit number.
 *      vid  - VLAN id to search for
 *      *stg - Spanning tree group id if found
 * Returns: 
 *      TRUE if the VLAN is mapped to an STG or FALSE if not
 */
extern int _bcm_fe2000_stg_map_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg);

/*
 * Function:
 *      bcm_fe2000_stg_default_get
 * Purpose:
 *      Returns the default STG for the device.
 * Parameters:
 *      unit    - device unit number.
 *      stg_ptr - STG ID for default.
 * Returns:
 *      BCM_E_XXX
 */
extern int bcm_fe2000_stg_default_get(int unit, bcm_stg_t *stg_ptr);

/*
 * Function:
 *      bcm_fe2000_stg_vlan_add
 * Purpose:
 *      Add a VLAN to a spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to use
 *      vid  - VLAN id to be added to STG
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Spanning tree group ID must have already been created. The 
 *      VLAN is removed from the STG it is currently in.
 */
extern int bcm_fe2000_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid);

/*
 * Function:
 *      bcm_fe2000_stg_stp_set
 * Purpose:
 *      Set the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - Spanning Tree State of port.
 * Returns:
 *      BCM_E_XXX
 */
extern int bcm_fe2000_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state);

/*
 * Function:
 *      bcm_fe2000_stg_stp_get
 * Purpose:
 *      Get the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - (Out) Pointer to where Spanning Tree State is stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
extern int bcm_fe2000_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state);

#endif	/* _BCM_INT_SBX_STG_H_ */
