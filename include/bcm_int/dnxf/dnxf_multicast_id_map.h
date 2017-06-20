/*
 * $Id: dnxf_multicast_id_map.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXF MULTICAST ID MAP H
 */
 
#ifndef _BCM_DNXF_MULTICAST_ID_MAP_H_
#define _BCM_DNXF_MULTICAST_ID_MAP_H_

#include <bcm/types.h>
#include <soc/dnxf/cmn/dnxf_defs.h>

int bcm_dnxf_multicst_id_map_init(int unit, soc_dnxf_multicast_table_mode_t mc_mode);
int bcm_dnxf_multicst_id_map_clear_id(int unit, int id);
int bcm_dnxf_multicst_id_map_mark_id(int unit, int id);
int bcm_dnxf_multicst_id_map_is_legal_id(int unit, bcm_multicast_t id, int* is_legal);
int bcm_dnxf_multicst_id_map_is_free_id(int unit, int id, int* is_free);
int bcm_dnxf_multicst_id_map_get_free_id(int unit, bcm_multicast_t* id);


#endif /*_BCM_DNXF_MULTICAST_ID_MAP_H_*/
