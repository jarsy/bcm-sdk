/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON_FE1600 STACK H
 */
 
#ifndef _SOC_RAMON_FE1600_STACK_H_
#define _SOC_RAMON_FE1600_STACK_H_

#include <bcm/stat.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/error.h>

soc_error_t soc_ramon_fe1600_stk_modid_set(int unit, uint32 fe_id);
soc_error_t soc_ramon_fe1600_stk_modid_get(int unit, uint32* fe_id);

#endif /*_SOC_RAMON_FE1600_STACK_H_*/
