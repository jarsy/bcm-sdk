/*
 * $Id: bcm_sw_db.c,v 1.20 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BCM_DBG_NORMAL
#include <shared/bsl.h>
#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/scache.h>
#include <bcm/module.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>

#ifdef BCM_WARM_BOOT_SUPPORT

char *g_bcm_sbx_caladan3_module_names[] = BCM_CALADAN3_MODULE_NAMES_INITIALIZER;

char *
bcm_caladan3_module_name(int unit, int module_num)
{
    if (module_num < BCM_MODULE__COUNT)
    {
        return bcm_module_name(unit, module_num);
    }

    if (sizeof(g_bcm_sbx_caladan3_module_names) / sizeof(g_bcm_sbx_caladan3_module_names[0])
                                                    != (BCM_CALADAN3_MODULE__COUNT - BCM_MODULE__COUNT)) {
        int i;

        i = sizeof(g_bcm_sbx_caladan3_module_names) / sizeof(g_bcm_sbx_caladan3_module_names[0]) - 1;

        LOG_ERROR(BSL_LS_APPL_SHELL,
                  (BSL_META_U(unit,
                              "bcm_module_name: BCM_MODULE_NAMES_INITIALIZER(%d) and BCM_MODULE__COUNT(%d) mis-match\n"), i, BCM_MODULE__COUNT));
        for(;i >= 0;i--) {
            LOG_CLI((BSL_META_U(unit,
                                "%2d. module_name %s module_num %d\n"), i, g_bcm_sbx_caladan3_module_names[i], (i+BCM_MODULE__COUNT)));
        }
    }

    if (module_num >= BCM_MODULE__COUNT && module_num < BCM_CALADAN3_MODULE__COUNT) 
    {
        return g_bcm_sbx_caladan3_module_names[(module_num - BCM_MODULE__COUNT)];
    }
    else
    {
        return "UNKNOWN";
    }

}

int
bcm_caladan3_scache_ptr_get(int unit, soc_scache_handle_t handle, soc_caladan3_scache_oper_t oper,
                                 int flags, uint32 *size, uint8 **scache_ptr,
                                 uint16 version, uint16 *recovered_ver, int *already_exists)
{
    if (oper == socScacheCreate) {
        LOG_CLI((BSL_META_U(unit,
                        "MODULE %s  MODULE ID %d: "), 
             bcm_caladan3_module_name(unit, SOC_SCACHE_HANDLE_MODULE_GET(handle)), SOC_SCACHE_HANDLE_MODULE_GET(handle)));
    }

    return soc_caladan3_scache_ptr_get(unit, handle, oper,
                                 flags, size, scache_ptr,
                                 version, recovered_ver, already_exists);

}

#endif /*BCM_WARM_BOOT_SUPPORT*/

#undef _ERR_MSG_MODULE_NAME

/*need to add flag mechanism to sw db*/
