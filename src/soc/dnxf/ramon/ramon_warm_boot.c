/*
 * $Id: ramon_fabric_status.c,v 1.9.48.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON WARM BOOT
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/error.h>

#ifdef BCM_88790_A0

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_warm_boot.h>

#include <soc/dnxf/ramon/ramon_warm_boot.h>

soc_error_t
soc_ramon_warm_boot_buffer_id_supported_get(int unit, int buffer_id, int *is_supported)
{
    DNXC_INIT_FUNC_DEFS;

    switch (buffer_id)
    {
        case SOC_DNXF_WARM_BOOT_BUFFER_MODID:
        case SOC_DNXF_WARM_BOOT_BUFFER_MC:
        case SOC_DNXF_WARM_BOOT_BUFFER_INTR:
		case SOC_DNXF_WARM_BOOT_BUFFER_PORT:
		case SOC_DNXF_WARM_BOOT_BUFFER_ISOLATE:
            *is_supported = 1;
            break;
        default:
            *is_supported = 0;
            break;
    }

    DNXC_FUNC_RETURN;
}

#endif /*BCM_88790_A0*/

#undef _ERR_MSG_MODULE_NAME
