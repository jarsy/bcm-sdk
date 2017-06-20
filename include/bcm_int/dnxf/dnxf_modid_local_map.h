/*
 * $Id: dnxf_modid_local_map.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DNXF MODID LOCAL MAP H
 */
 
#ifndef _BCM_DNXF_MODID_LOCAL_MAP_H_
#define _BCM_DNXF_MODID_LOCAL_MAP_H_

#include <bcm/types.h>

#include <soc/dnxf/cmn/dnxf_drv.h>

#define DNXF_MODID_LOCAL_MAP_ROWS(unit)  (SOC_DNXF_MODID_LOCAL_NOF(unit))

#define DNXF_MODID_LOCAL_MAP_ROW_VALIDATE(unit, raw) (((raw < DNXF_MODID_LOCAL_MAP_ROWS(unit)) && (raw > -1)) ? 1 : 0)

typedef  soc_dnxf_modid_local_map_t _bcm_dnxf_modid_local_map_t;

int bcm_dnxf_modid_local_map_clear(int unit);
int bcm_dnxf_modid_local_map_module_clear(int unit, bcm_module_t local_module_id);
int bcm_dnxf_modid_local_map_is_valid(int unit, bcm_module_t local_module_id, int* is_valid);
int bcm_dnxf_modid_local_map_add(int unit, bcm_module_t local_module_id, bcm_module_t module_id, int allow_override);
int bcm_dnxf_modid_local_map_get(int unit, bcm_module_t local_module_id, bcm_module_t* module_id);
int bcm_dnxf_modid_local_map_modid_to_local(int unit, bcm_module_t module_id, bcm_module_t* local_module_id);


#endif /*_BCM_DNXF_MODID_LOCAL_MAP_H_*/
