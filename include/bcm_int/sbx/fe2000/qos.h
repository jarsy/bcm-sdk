/*
 * $Id: qos.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qos.h
 * Purpose:     qos internally exported interface
 *              specific to FE2000 BCM API implementation
 */

#ifndef _BCM_INT_SBX_FE2000_QOS_H_
#define _BCM_INT_SBX_FE2000_QOS_H_

#include <bcm/qos.h>

/*
 *  We must differentiate by ID the ingress versus egress QoS maps.  This is
 *  because they are different, and the settings are managed by ID only.
 *
 *  This value is added to the ID on an egress QoS map.
 */
#define QOS_MAP_ID_EGRESS_OFFSET 0x00001000

/*
 *   Function
 *      _bcm_fe2000_qos_map_id_to_hw_id
 *   Purpose
 *      temporary translation api for use by port while its qos
 *      api is still suppported.
 *   Parameters
 *      (in)  flags          - BCM_QOS_MAP_[INGRESS|EGRESS]
 *      (in)  map_id         - qos map id to be translated.
 *   Returns
 *       hardware index for given resource
 */
int _bcm_fe2000_qos_map_id_to_hw_id(int flags, int map_id);

/*
 *   Function
 *      _bcm_fe2000_qos_hw_id_to_map_id
 *   Purpose
 *      temporary translation api for use by port while its qos
 *      api is still suppported.
 *   Parameters
 *      (in)  flags          - BCM_QOS_MAP_[INGRESS|EGRESS]
 *      (in)  hw_id          - hardware id to be translated.
 *   Returns
 *       qos map id for given resource
 */
int _bcm_fe2000_qos_hw_id_to_map_id(int flags, int hw_id);

#endif
