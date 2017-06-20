/*
 * $Id: dnxf_port.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXF warm_boot
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_INIT

#include <shared/bsl.h>
#include <soc/error.h>
#include <soc/wb_engine.h>
#include <soc/scache.h>
#include <soc/ha.h>

#include <sal/compiler.h>

#ifdef BCM_DNXF_SUPPORT

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_warm_boot.h>


#define DNXF_VER(_ver) (_ver)


soc_error_t
soc_dnxf_warm_boot_sync(int unit)
{
    DNXC_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    DNXC_IF_ERR_EXIT(soc_wb_engine_sync(unit, SOC_DNXF_WARM_BOOT_ENGINE));
#else
    DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Warm boot support requires compilation with warm boot flag.\n")));
#endif /*BCM_WARM_BOOT_SUPPORT*/

exit:  
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxf_warm_boot_deinit(int unit)
{
    int rv;
#if defined(BCM_WARM_BOOT_SUPPORT) && !defined(__KERNEL__) && defined (LINUX)
    int stable_location;
    uint32 stable_flags;
#endif

    DNXC_INIT_FUNC_DEFS;
    
    rv = soc_wb_engine_deinit_tables(unit, SOC_DNXF_WARM_BOOT_ENGINE);
    DNXC_IF_ERR_EXIT(rv);

#ifdef BCM_WARM_BOOT_SUPPORT
    rv = soc_scache_detach(unit);
    DNXC_IF_ERR_EXIT(rv);
#if !defined(__KERNEL__) && defined (LINUX)
    DNXC_IF_ERR_RETURN(soc_stable_get(unit,&stable_location,&stable_flags));
    if (stable_location == 4) {
        rv = ha_destroy(unit, 0);
        DNXC_IF_ERR_CONT(rv);
    }
#endif
#endif

exit: 
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxf_warm_boot_init(int unit)
{
    int rv,
        is_supported,
        buffer_id;
#if defined(BCM_WARM_BOOT_SUPPORT) && !defined(__KERNEL__) && defined (LINUX)
    int stable_location;
    uint32 stable_flags;
    uint32 stable_size;
#endif

    DNXC_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
#if !defined(__KERNEL__) && defined (LINUX)
    stable_size = soc_property_get(unit, spn_STABLE_SIZE, 100000000);

    /* create new file if not SOC_WARM_BOOT */
    DNXC_IF_ERR_RETURN(soc_stable_get(unit,&stable_location,&stable_flags));
    if (stable_location == 4) {
        /* init HA only if stable location is shared memory (4) */
        DNXC_IF_ERR_RETURN(ha_init(unit, 1 /* HA enabled */, "HA", stable_size, SOC_WARM_BOOT(unit) ? 0 : 1));
    }
#endif

    /* Recover stored Level 2 Warm Boot cache */
    /* The stable and stable size must be selected first */
    if (SOC_WARM_BOOT(unit)) 
    {
        rv = soc_scache_recover(unit);
        if (SOC_FAILURE(rv)) 
        {
            /* Fall back to Level 1 Warm Boot recovery */
            DNXC_IF_ERR_RETURN(soc_stable_size_set(unit, 0));
            DNXC_IF_ERR_RETURN(soc_stable_set(unit, _SHR_SWITCH_STABLE_NONE, 0));
            /* Error report */
            DNXC_IF_ERR_RETURN
                (soc_event_generate(unit, SOC_SWITCH_EVENT_STABLE_ERROR, 
                                    SOC_STABLE_CORRUPT,
                                    SOC_STABLE_FLAGS(unit), 0));
            LOG_VERBOSE(BSL_LS_SOC_INIT,
                        (BSL_META_U(unit,
                                    "Unit %d: Corrupt stable cache.\n"),
                                    unit));
        }
    }
#endif  

    DNXC_IF_ERR_EXIT(soc_wb_engine_init_tables(unit, SOC_DNXF_WARM_BOOT_ENGINE , SOC_DNXF_WARM_BOOT_BUFFER_NOF, SOC_DNXF_WARM_BOOT_VAR_NOF)); 

    /*Build engine structure per buffer*/
    for (buffer_id = 0; buffer_id < SOC_DNXF_WARM_BOOT_BUFFER_NOF; buffer_id++)
    {
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_warm_boot_buffer_id_supported_get, (unit, buffer_id, &is_supported));
        DNXC_IF_ERR_EXIT(rv);

        if (is_supported)
        {
            rv = soc_dnxf_warm_boot_buffer_id_create(unit, buffer_id);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    
    rv = SOC_DNXF_WARM_BOOT_VAR_GET(unit, INTR_STORM_NOMINAL, &SOC_SWITCH_EVENT_NOMINAL_STORM(unit));
    DNXC_IF_ERR_EXIT(rv);


exit: 
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxf_warm_boot_buffer_id_create(int unit, int buffer_id)
{
    int rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;

    /* 
     * init the buffer info and buffer's variables info in wb engine tables, 
     * each buffer init it's own data. 
     * note that the location of original data is different in each boot.
     */ 
    rc = soc_dnxf_warm_boot_engine_init_buffer_struct(unit, buffer_id);
    DNXC_IF_ERR_EXIT(rc);

    rc = soc_wb_engine_init_buffer(unit, SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, FALSE);
    if (rc == SOC_E_NOT_FOUND) {
        
        /*Fall back - check if it is a new buffer and it is ISSU procedure*/
        LOG_ERROR(BSL_LS_BCM_INIT,
                  (BSL_META_U(unit,
                              "Failed to recover buffer (%d) warm boot data"), buffer_id));

        
        DNXC_IF_ERR_EXIT(rc);
    } else {
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_dnxf_warm_boot_engine_init_buffer_struct(int unit, int buffer_id)
{
    int rv,
        nof_links,
        nof_macs,
        nof_interrupts;
    uint32 mc_table_size;
    WB_ENGINE_INIT_TABLES_DEFS;
    DNXC_INIT_FUNC_DEFS;

    COMPILER_REFERENCE(&buffer_is_dynamic); /*unused var*/

    /*Get info*/
    nof_links = SOC_DNXF_DEFS_GET(unit, nof_links);
    nof_macs = SOC_DNXF_DEFS_GET(unit, nof_instances_mac);
    nof_interrupts = SOC_DNXF_DEFS_GET(unit, nof_interrupts);
    rv = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_multicast_table_size_get,(unit, &mc_table_size));
    DNXC_IF_ERR_EXIT(rv);

    switch (buffer_id)
    {
        case SOC_DNXF_WARM_BOOT_BUFFER_MODID:
            /*Create buffer*/
            SOC_WB_ENGINE_ADD_BUFF(SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, "Fabric modid buffer", NULL, NULL, DNXF_VER(1), 1, SOC_WB_ENGINE_PRE_RELEASE);
            DNXC_IF_ERR_EXIT(rv);

            /*Create Vars*/
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_MODID_LOCAL_MAP,          "Local modids mapping",             buffer_id, sizeof(soc_dnxf_modid_local_map_t),   NULL, SOC_DNXF_MODID_LOCAL_NOF(unit),        DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_MODID_GROUP_MAP,          "Group modids mapping",             buffer_id, sizeof(soc_dnxf_modid_group_map_t),   NULL, SOC_DNXF_MODID_GROUP_NOF(unit),        DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_MODID_MODID_TO_GROUP_MAP, "Modid to group modids mapping",    buffer_id, sizeof(soc_module_t),                NULL, SOC_DNXF_MODID_NOF,                    DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            break;

        case SOC_DNXF_WARM_BOOT_BUFFER_MC:
            /*Create buffer*/
            SOC_WB_ENGINE_ADD_BUFF(SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, "MC buffer", NULL, NULL, DNXF_VER(1), 1, SOC_WB_ENGINE_PRE_RELEASE);
            DNXC_IF_ERR_EXIT(rv);

            /*Create Vars*/
            SOC_WB_ENGINE_ADD_VAR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_MC_MODE,                   "MC Mode",                          buffer_id, sizeof(uint32),                      NULL,                                       DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_MC_ID_MAP,                "MC_MAP",                           buffer_id, sizeof(uint32),                      NULL, _shr_div32r(mc_table_size, 32),       DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            break;

        case SOC_DNXF_WARM_BOOT_BUFFER_FIFO:
            /*Create buffer*/
            SOC_WB_ENGINE_ADD_BUFF(SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, "FIFO handles", NULL, NULL, DNXF_VER(1), 1, SOC_WB_ENGINE_PRE_RELEASE);
            DNXC_IF_ERR_EXIT(rv);

            /*Create Vars*/
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_FIFO_HANDLERS,             "Fifo handles",                     buffer_id, sizeof(soc_dnxf_fifo_type_handle_t),  NULL, soc_dnxf_fabric_nof_link_fifo_types,  DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            break;

        case SOC_DNXF_WARM_BOOT_BUFFER_PORT:
            /*Create buffer*/
            SOC_WB_ENGINE_ADD_BUFF(SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, "Port params", NULL, NULL, DNXF_VER(1), 1, SOC_WB_ENGINE_PRE_RELEASE);
            DNXC_IF_ERR_EXIT(rv);

            /*Create Vars*/
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_PORT_COMMA_BURST_CONF,    "Port Comma burst",                 buffer_id, sizeof(uint32),                      NULL, nof_links,                            DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_PORT_CTRL_BURST_CONF,     "Port Control burst",               buffer_id, sizeof(uint32),                      NULL, nof_macs,                             DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_PORT_CL72_CONF,           "Port CL72",                        buffer_id, sizeof(uint32),                      NULL, nof_links,                            DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            break;

       case SOC_DNXF_WARM_BOOT_BUFFER_INTR:
            /*Create buffer*/
            SOC_WB_ENGINE_ADD_BUFF(SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, "Interrupts params", NULL, NULL, DNXF_VER(1), 1, SOC_WB_ENGINE_PRE_RELEASE);
            DNXC_IF_ERR_EXIT(rv);

            /*Create Vars*/
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_INTR_FLAGS,               "intr flags",                       buffer_id, sizeof(uint32),                      NULL, nof_interrupts,                       DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_INTR_STORM_TIMED_COUNT,   "intr storm timed count",           buffer_id, sizeof(uint32),                      NULL, nof_interrupts,                       DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_ARR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_INTR_STORM_TIMED_PERIOD,  "intr storm timed period",          buffer_id, sizeof(uint32),                      NULL, nof_interrupts,                       DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            SOC_WB_ENGINE_ADD_VAR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_INTR_STORM_NOMINAL,       "intr storm nominal",               buffer_id, sizeof(uint32),                      NULL,                                       DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);
            break;
        case SOC_DNXF_WARM_BOOT_BUFFER_ISOLATE:
            /*Create buffer*/
            SOC_WB_ENGINE_ADD_BUFF(SOC_DNXF_WARM_BOOT_ENGINE, buffer_id, "Isolation params", NULL, NULL, DNXF_VER(1), 1, SOC_WB_ENGINE_PRE_RELEASE);
            DNXC_IF_ERR_EXIT(rv);
            
            /*Create Vars*/
            SOC_WB_ENGINE_ADD_VAR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_ISOLATE_UNISOLATED_LINKS, "Unisolated Links bitmap",         buffer_id , sizeof(soc_pbmp_t),                  NULL,                                         DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);    
            SOC_WB_ENGINE_ADD_VAR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_ISOLATE_ACTIVE_LINKS,     "Active Links bitmap",             buffer_id , sizeof(soc_pbmp_t),                  NULL,                                         DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);    
            SOC_WB_ENGINE_ADD_VAR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_ISOLATE_ISOLATE_DEVICE,   "Isolate device flag",             buffer_id , sizeof(soc_dnxc_isolation_status_t), NULL,                                         DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);    
            SOC_WB_ENGINE_ADD_VAR(SOC_DNXF_WARM_BOOT_ENGINE, SOC_DNXF_WARM_BOOT_VAR_ISOLATE_TYPE,             "Valid isolation type",            buffer_id , sizeof(uint32),                      NULL,                                         DNXF_VER(1));
            DNXC_IF_ERR_EXIT(rv);

            break;

        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INIT, (_BSL_DNXC_MSG("Unsupported buffer_id %d"), buffer_id));
            break;
    }

    SOC_WB_ENGINE_INIT_TABLES_SANITY(rv);

exit:
    DNXC_FUNC_RETURN;
}

#endif /*BCM_DNXF_SUPPORT*/
