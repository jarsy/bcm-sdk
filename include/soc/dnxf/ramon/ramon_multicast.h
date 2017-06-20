/*
 * $Id: ramon_multicast.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON MULTICAST H
 */
 
#ifndef _SOC_RAMON_MULTICAST_H_
#define _SOC_RAMON_MULTICAST_H_

#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/error.h>

soc_error_t soc_ramon_multicast_mode_get(int unit, soc_dnxf_multicast_table_mode_t* multicast_mode);
soc_error_t soc_ramon_multicast_table_size_get(int unit, uint32* mc_table_size);
soc_error_t soc_ramon_multicast_table_entry_size_get(int unit, uint32* entry_size);

#endif

