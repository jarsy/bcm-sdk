/*
 * $Id: dos.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Field Processor related CLI commands
 */
#include <shared/bsl.h>

#include <sal/types.h>

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/drv_if.h>

/* DOS event monitor :
 *  - Monitor the HW DOS event status.
 */
typedef struct robo_dos_monitor_s {
    char    task_name[16];
    uint32  interval;
    uint32  err_cnt;
    uint32  last_dos_events;
    
    sal_mutex_t         dm_lock;
    sal_sem_t           dm_sema;
    VOL sal_thread_t    dm_thread;
}drv_robo_dos_monitor_t;

STATIC drv_robo_dos_monitor_t *drv_dm_control[SOC_MAX_NUM_SWITCH_DEVICES];

#define ROBO_HWDOS_MONITOR_PRI  50

/*
 * Define:
 *  DM_LOCK/DM_UNLOCK
 * Purpose:
 *  Serialization Macros for access to drv_robo_dos_monitor_t structure.
 */

#define DM_LOCK(unit) \
        sal_mutex_take(drv_dm_control[unit]->dm_lock, sal_mutex_FOREVER)

#define DM_UNLOCK(unit) \
        sal_mutex_give(drv_dm_control[unit]->dm_lock)

#define MIN_DRV_DOS_MONITOR_INTERVAL    10000
#define DM_MAX_ERR_COUNT                100

/*
 * Function:
 *  soc_robo_dos_monitor_thread
 * Purpose:
 *      DOS event monitor thread
 * Parameters:
 *  unit     - unit number.
 * Returns:
 *  
 */
STATIC void
soc_robo_dos_monitor_thread(int unit)
{
    drv_robo_dos_monitor_t  *dm = drv_dm_control[unit];
    int     rv = SOC_E_NONE;
    int     interval = 0;
    uint32  events_bmp = 0, hwdos_enable_bmp = 0;
    
    dm->dm_thread = sal_thread_self();
    
    while ((interval = dm->interval) != 0) {
        DM_LOCK(unit);
        
        if (dm->err_cnt < DM_MAX_ERR_COUNT){
            /* check HW DOS enable status : if no HW DOS, stop the thread.
             *  - HW DOS configuration could be modified by API or User  
             *      Application. This process will prevent dummy thread is 
             *      running for no HW DOS enabled.
             *  - Thread will be started agagin if any HW DOS enabled through
             *      switch API.
             */
            rv = DRV_DOS_EVENT_BITMAP_GET(unit, 
                    DRV_DOS_EVT_OP_ENABLED, &hwdos_enable_bmp);
            if (hwdos_enable_bmp == 0){
                /* means no HW DOS is enabled, stop thread !
                 *  - but still proceed whole process in case there are one 
                 *      or more DOS event occurred already.
                 */
                interval = sal_sem_FOREVER;
            }
            
            /* read HW DOS event */
            rv = DRV_DOS_EVENT_BITMAP_GET(unit, 
                    DRV_DOS_EVT_OP_STATUS, &events_bmp);
            if (rv){
                dm->err_cnt ++;
                if (dm->err_cnt == DM_MAX_ERR_COUNT){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s Thread stop for %d times failed processes.\n"),
                              dm->task_name, DM_MAX_ERR_COUNT));
                    /* generate an event action with THREAD error type.
                     *  - arg1 : indicate the error item.
                     *  - arg2 : indicate the error line.
                     *  - arg3 : indicate the error code.
                     */
                    soc_event_generate(unit, 
                            SOC_SWITCH_EVENT_THREAD_ERROR, 
                            SOC_SWITCH_EVENT_THREAD_HWDOS_MONITOR, 
                            __LINE__, rv);
                    dm->err_cnt = 0;
                    dm->last_dos_events = 0;
                    interval = sal_sem_FOREVER;
                }
            } else {
                if (events_bmp){
                    /* generate an event action and carry the DOS events in 
                     *  arg1.(arg2 and arg3 assigned 0 for no reference here)
                     */
                    soc_event_generate(unit, 
                            SOC_SWITCH_EVENT_DOS_ATTACK, 
                            events_bmp, 0, 0);
                    dm->last_dos_events = events_bmp;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "Check DOS event bmp=0x%x\n"), 
                                 events_bmp));
                }
                dm->err_cnt = 0;
            }
        }

        DM_UNLOCK(unit);
        (void)sal_sem_take(dm->dm_sema, interval);
    }
    
    dm->dm_thread = NULL;
    sal_thread_exit(0);
}

/*
 * Function:
 *  soc_robo_dos_monitor_enable_set (internal)
 * Purpose:
 *      Enable/disable DOS event monitor threads
 * Parameters:
 *  unit     - unit number.
 *  interval - time between resynchronization passes
 * Returns:
 *  SOC_E_INTERNAL if can't create threads.
 */
int
soc_robo_dos_monitor_enable_set(int unit, sal_usecs_t interval)
{
    drv_robo_dos_monitor_t  *dm = drv_dm_control[unit];
    sal_usecs_t     us = interval;
    soc_timeout_t   to;
    
    /* return if not init yet */
    if (dm == NULL){
        if (interval == 0){
            /* no error return if set thread disable when dm not init */
            return SOC_E_NONE;
        } else {
            /* init problem when enabling dm thread */
            return SOC_E_INIT;
        }
    }
    
    sal_snprintf(dm->task_name, sizeof(dm->task_name), 
            "robo_DOS_EVENT.%d", unit);
            
    if (us){
        /* --- enabling thread --- */
        us = (interval >= MIN_DRV_DOS_MONITOR_INTERVAL) ? us : 
                MIN_DRV_DOS_MONITOR_INTERVAL;
                
        dm->interval = us;
        if (dm->dm_thread != NULL){
            /* if thread is running, update the period and return */
            sal_sem_give(dm->dm_sema);
            return SOC_E_NONE;
        } else {
            if (sal_thread_create(dm->task_name, 
                    SAL_THREAD_STKSZ, 
                    ROBO_HWDOS_MONITOR_PRI,
                    (void (*)(void*))soc_robo_dos_monitor_thread,
                    INT_TO_PTR(unit)) == SAL_THREAD_ERROR){
                        
                dm->interval = 0;
                dm->err_cnt = 0;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Thread is not created\n")));
                soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                        SOC_SWITCH_EVENT_THREAD_HWDOS_MONITOR, 
                        __LINE__, SOC_E_MEMORY);
                return SOC_E_MEMORY;
            } else {
            
                soc_timeout_init(&to, 3000000, 0);
                while (dm->dm_thread == NULL) {
                    if (soc_timeout_check(&to)) {
                        dm->interval = 0;
                        dm->err_cnt = 0;
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: Thread did not start\n"),
                                   dm->task_name));
                        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                            SOC_SWITCH_EVENT_THREAD_HWDOS_MONITOR, 
                            __LINE__, SOC_E_INTERNAL);
                        return SOC_E_INTERNAL;
                        break;
                    }
                }
            }
        }
        
    } else {
        /* disabling thread */
        dm->interval = 0;
        
        sal_sem_give(dm->dm_sema);
        
        soc_timeout_init(&to, 3000000, 0);
        while (dm->dm_thread != NULL) {
            if (soc_timeout_check(&to)) {
                dm->interval = 0;
                dm->err_cnt = 0;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: Thread did not exit\n"),
                           dm->task_name));
                soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                    SOC_SWITCH_EVENT_THREAD_HWDOS_MONITOR, 
                    __LINE__, SOC_E_INTERNAL);
                return SOC_E_INTERNAL;
                break;
            }
        }
    }
    
    /* set interval if changed */
    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_robo_dos_monitor_enable_get
 * Purpose:
 *      Get the DOS event monitor enable/disable status
 * Parameters:
 *  unit     - unit number.
 *  interval - (OUT)time between resynchronization passes
 * Returns:
 *  
 */
int
soc_robo_dos_monitor_enable_get(int unit, sal_usecs_t *interval)
{
    /* return interval=0 if not init yet */
    if (drv_dm_control[unit] == NULL) {
        *interval = 0;
    } else {
        /* return current interval */
        *interval = drv_dm_control[unit]->interval;
    }
    
    return SOC_E_NONE;

}

/*
 * Function:
 *  soc_robo_dos_monitor_enable_get
 * Purpose:
 *      Get the DOS event monitor enable/disable status
 * Parameters:
 *  unit     - unit number.
 *  interval - (OUT)time between resynchronization passes
 * Returns:
 *  
 */
int
soc_robo_dos_monitor_last_event(int unit, uint32 *events_bmp)
{
    /* return no events if not init yet */
    if (drv_dm_control[unit] == NULL) {
        *events_bmp = 0;
    } else {
        /* return current interval */
        *events_bmp = drv_dm_control[unit]->last_dos_events;
    }
    
    return SOC_E_NONE;

}

/*
 *  soc_robo_dos_monitor_deinit
 * Purpose:
 *      dos monitor de-init
 * Parameters:
 *  unit     - unit number.
 * Returns:
 *  
 */
int
soc_robo_dos_monitor_deinit(int unit)
{
    drv_robo_dos_monitor_t  *dm = drv_dm_control[unit];
    
    /* check if dm is null and dm thread running status */
    if (dm != NULL){
        
        SOC_IF_ERROR_RETURN(soc_robo_dos_monitor_enable_set(unit, 0));
        
        if (dm->dm_thread != NULL){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "HW DOS monitor disabled but still exist!,thread(%p)\n"),
                       (void *)dm->dm_thread));
        }
        
        if (dm->dm_sema != NULL) {
            sal_sem_destroy(dm->dm_sema);
            dm->dm_sema = NULL;
        }
        
        if (dm->dm_lock != NULL) {
            sal_mutex_destroy(dm->dm_lock);
            dm->dm_lock = NULL;
        }
        
        /* free dm  */
        drv_dm_control[unit] = NULL;
        sal_free(dm);
    }
    
    return SOC_E_NONE;
    
}

/*
 * Function:
 *  soc_robo_dos_monitor_init (internal)
 * Purpose:
 *      dos monitor init
 * Parameters:
 *  unit     - unit number.
 *  interval - time between resynchronization passes
 * Returns:
 *  SOC_E_INTERNAL if can't create threads.
 */
int
soc_robo_dos_monitor_init(int unit)
{
    drv_robo_dos_monitor_t  *dm;
    
    if (drv_dm_control[unit] != NULL){
        SOC_IF_ERROR_RETURN(soc_robo_dos_monitor_deinit(unit));
    }
    if ((dm = sal_alloc(sizeof(drv_robo_dos_monitor_t), "dos_monitor")) == 
            NULL){
        return SOC_E_MEMORY;
    }
    
    /* allocate dm and set init value */
    sal_memset(dm, 0, sizeof (drv_robo_dos_monitor_t));
    
    dm->dm_lock = sal_mutex_create("soc_dos_monitor_lock");
    if (dm->dm_lock == NULL){
        sal_free(dm);
        return SOC_E_MEMORY;
    }

    dm->dm_sema = sal_sem_create("robo_HWDOS_MONITOR_SLEEP", 
                 sal_sem_BINARY, 0);
    if (dm->dm_sema == NULL) {
        sal_mutex_destroy(dm->dm_lock);
        sal_free(dm);
        return SOC_E_MEMORY;
    }
    dm->dm_thread = NULL;
    
    drv_dm_control[unit] = dm;

    return SOC_E_NONE;
}

