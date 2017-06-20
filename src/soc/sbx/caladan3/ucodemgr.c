/*
 * $Id: ucodemgr.c,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    ucodemgr.c
 * Purpose: Ucode image download manager
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <sal/appl/sal.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/asm3/asm3_pkg_intf.h>
#include <soc/sbx/caladan3/asm3/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/simintf.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
extern int ASM_INIT_FUNC(g3p1)(soc_sbx_caladan3_ucode_pkg_t*);
extern int ASM_INIT_FUNC(g3p1a)(soc_sbx_caladan3_ucode_pkg_t*);
#endif

#ifdef BCM_CALADAN3_T3P1_SUPPORT
#include <soc/sbx/t3p1/t3p1.h>
extern int ASM_INIT_FUNC(t3p1)(soc_sbx_caladan3_ucode_pkg_t*);
extern int ASM_INIT_FUNC(t3p1a)(soc_sbx_caladan3_ucode_pkg_t*);
#endif


/*
 * LRP Ucode download manager
 */

soc_sbx_caladan3_ucodemgr_t _ucodemgr[SOC_MAX_NUM_DEVICES];
extern sal_mutex_t bankSwapLock[SOC_MAX_NUM_DEVICES];

#define UCODEMGR_LOCK(unit) \
             sal_mutex_take(_ucodemgr[(unit)].lock, sal_mutex_FOREVER)

#define UCODEMGR_UNLOCK(unit) \
             sal_mutex_give(_ucodemgr[(unit)].lock)

#define UCODEMGR_CHECKALL(u) ((u)->asm_init && \
           (u)->app_init && (u)->lr_download && \
           (u)->lr_prepare && (u)->lr_done &&   \
           (u)->lr_iread && (u)->lr_iwrite)

soc_sbx_caladan3_ucodemgr_t *
soc_sbx_caladan3_lr_ucodemgr_get(int unit) {
    if (unit < SOC_MAX_NUM_DEVICES)
        return &(_ucodemgr[unit]);
    else
        return NULL;
}

int 
soc_sbx_caladan3_lr_ucodemgr_init(int unit) 
{

    sal_mutex_t lock;
    soc_sbx_caladan3_ucodemgr_t *umgr;
    int i, rv;

    umgr = &_ucodemgr[unit];

    if (umgr->lock == NULL) {
	lock = sal_mutex_create("C3_UCODE_MGR_MUTEX");
	if (lock == NULL) {
	    return SOC_E_RESOURCE;
	}
	sal_memset(umgr, 0, sizeof(*umgr));
	umgr->lock = lock;
    }

    /* Init with defaults */
    rv = SOC_E_NONE;
    umgr->app_init = soc_sbx_caladan3_ucodemgr_app_init;
    umgr->asm_init = soc_sbx_caladan3_ucodemgr_asm_init;
    umgr->lr_prepare = soc_sbx_caladan3_lr_ucode_prepare;
    umgr->lr_download = soc_sbx_caladan3_lr_download;
    umgr->lr_done = soc_sbx_caladan3_lr_ucode_done;
    umgr->lr_iread = soc_sbx_caladan3_lr_iread;
    umgr->lr_iwrite = soc_sbx_caladan3_lr_iwrite;
    umgr->bank = 0;
    umgr->allbanks = 1;

    for(i=0; i < SOC_SBX_CALADAN3_LR_INST_NUM_BANKS; i++) 
        umgr->dirty[i] = BANK_INVALID;

    rv = soc_sbx_caladan3_ucode_debug_thread_start(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Debug Init Failed\n")));
    }

    /* Add Microcode application registration here */
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    soc_sbx_caladan3_ucodemgr_register_app(unit, "g3p1", g3p1_app_init, ASM_INIT_FUNC(g3p1));

    soc_sbx_caladan3_ucodemgr_register_app(unit, "g3p1a", g3p1_app_init, ASM_INIT_FUNC(g3p1a));
#endif
#ifdef BCM_CALADAN3_T3P1_SUPPORT
    soc_sbx_caladan3_ucodemgr_register_app(unit, "t3p1", t3p1_app_init, ASM_INIT_FUNC(t3p1));

    soc_sbx_caladan3_ucodemgr_register_app(unit, "t3p1a", t3p1_app_init, ASM_INIT_FUNC(t3p1a));
#endif

    /* 
    extern int ASM_INIT_FUNC(custom)(soc_sbx_caladan3_ucode_pkg_t*);
    soc_sbx_caladan3_ucodemgr_register_app(unit, "custom", custom_app_init, ASM_INIT_FUNC(custom));
    */

    return rv;

}


/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_app_init
 * Description:
 *    Routine to initialize the microcode application 
 *    This is called after an image download is successful
 */
int
soc_sbx_caladan3_ucodemgr_app_init(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg)
{
    int rv = SOC_E_NONE;
    int flags = 0;
    soc_sbx_caladan3_ucodemgr_t *umgr;
    soc_sbx_control_t *sbx;
    soc_sbx_caladan3_lrp_t *lrp;
    uint32 epoch, contexts;
    char *ucodestr;
    APP_INIT_F app_init;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }
    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp) {
        return SOC_E_INIT;
    }

    umgr = &_ucodemgr[unit];
    flags = umgr->reload;

    ucodestr = soc_property_get_str(unit, spn_BCM88030_UCODE);
    if (ucodestr == NULL) {
        /* Broadcom standard application */
        ucodestr = "g3p1";
    }

    app_init = soc_sbx_caladan3_ucodemgr_find_app(unit, ucodestr);
    if (app_init) {
        rv = app_init(unit, pkg, &contexts, &epoch, flags);
    } else {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "soc_sbx_caladan3_ucodemgr_loadimg: unit %d No ucode %s detected \n"),
                  unit, ucodestr));
       return SOC_E_PARAM;
    }

    if (SOC_SUCCESS(rv)) {

        if ((epoch >= SOC_SBX_CALADAN3_LR_EPOCH_LENGTH_MIN) &&
            (epoch < SOC_SBX_CALADAN3_LR_EPOCH_LENGTH_MAX)) {
            lrp->epoch_length = epoch;
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d Invalid Epoch len \n"), unit));
            return SOC_E_PARAM;
        }

        if (contexts <= SOC_SBX_CALADAN3_LR_CONTEXTS_USER_MAX) {
            lrp->num_context = contexts;
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d Invalid Number of context switches \n"), unit));
            return SOC_E_PARAM;
        }
    }

    return rv;
}


/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_asm_init
 * Description:
 *    Routine to initialize the Asm2 interfaces
 */
int
soc_sbx_caladan3_ucodemgr_asm_init(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg)
{
    /* Register Lr instruction read write callbacks */
    soc_sbx_caladan3_ucodemgr_t *umgr;
    PKG_INIT_F asm_init = NULL;
    char *ucodestr = NULL;
    int hwid;
    int revid;
    uint16 g_dev_id = 0;
    uint8  g_rev_id = 0;

    umgr = &_ucodemgr[unit];

    ucodestr = soc_property_get_str(unit, spn_BCM88030_UCODE);
    if (ucodestr == NULL) {
        /* Broadcom standard application */
        ucodestr = "g3p1";
    }
    asm_init = soc_sbx_caladan3_ucodemgr_find_asm_init(unit, ucodestr);
    if (asm_init) {
        asm_init(pkg);
        soc_cm_get_id( unit, &g_dev_id, &g_rev_id);
        hwid = (pkg->m_hwid & 0xfff);
        revid = (pkg->m_hwid >> 12) & 0xff;

	/* coverity[dead_error_line] */
        if ((hwid == BCM88030_DEVICE_ID && 
             (revid == BCM88030_A0_REV_ID || revid == BCM88030_A1_REV_ID) &&
             (g_dev_id != BCM88030_DEVICE_ID || 
              (g_rev_id != BCM88030_A0_REV_ID && g_rev_id != BCM88030_A1_REV_ID))) ||
            (hwid == BCM88030_DEVICE_ID && revid == BCM88030_B0_REV_ID &&
             (g_dev_id != BCM88030_DEVICE_ID || g_rev_id != BCM88030_B0_REV_ID)) ||
            (hwid == BCM88030_DEVICE_ID && revid == BCM88030_B1_REV_ID &&
             (g_dev_id != BCM88030_DEVICE_ID || g_rev_id != BCM88030_B1_REV_ID)) ||
            (hwid == BCM88034_DEVICE_ID && 
             (revid == BCM88034_A0_REV_ID || revid == BCM88034_A1_REV_ID) &&
             (g_dev_id != BCM88034_DEVICE_ID || 
              (g_rev_id != BCM88034_A0_REV_ID || g_rev_id != BCM88034_A1_REV_ID)))) {

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "unit %d Ucode target hardware missmatch, ucode is built for dev:%d rev:%d  hardware is dev:%d rev:%d\n"), 
                       unit, hwid, revid, g_dev_id, g_rev_id));
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "unit %d Ucode application may not run optimally or may experience errors.\n"), unit));
        }

        C3Asm3__PkgInt__initF(pkg, umgr->lr_iwrite, umgr->lr_iread, INT_TO_PTR(unit));
    }
  
    return SOC_E_NONE;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_find_app
 * Description:
 *    Find the app handler for this app type
 */
PKG_INIT_F
soc_sbx_caladan3_ucodemgr_find_asm_init(int unit, char* type)
{
    int i;
    soc_sbx_caladan3_ucodemgr_t *umgr;

    umgr = &_ucodemgr[unit];
    UCODEMGR_LOCK(unit);

    for (i=0; i < MAX_UCODE_APPS; i++) {
        if (sal_strncasecmp(umgr->app_list[i].type, type, 16) == 0) {
             UCODEMGR_UNLOCK(unit);
             return umgr->app_list[i].asminit;
        }
    }
    UCODEMGR_UNLOCK(unit);
    return NULL;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_find_app
 * Description:
 *    Find the app handler for this app type
 */
APP_INIT_F
soc_sbx_caladan3_ucodemgr_find_app(int unit, char* type)
{
    int i;
    soc_sbx_caladan3_ucodemgr_t *umgr;

    umgr = &_ucodemgr[unit];
    UCODEMGR_LOCK(unit);

    for (i=0; i < MAX_UCODE_APPS; i++) {
        if (sal_strncasecmp(umgr->app_list[i].type, type, 16) == 0) {
             UCODEMGR_UNLOCK(unit);
             return umgr->app_list[i].handler;
        }
    }
    UCODEMGR_UNLOCK(unit);
    return NULL;
}


/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_register_app
 * Description:
 *    Routine to register a callback. This can be used to override defaults
 */
int
soc_sbx_caladan3_ucodemgr_register_app(int unit, char* type, APP_INIT_F func, PKG_INIT_F pkgf)
{
    int i;
    soc_sbx_caladan3_ucodemgr_t *umgr;
    int rv = SOC_E_NONE;

    umgr = &_ucodemgr[unit];
    UCODEMGR_LOCK(unit);

    for (i=0; i < MAX_UCODE_APPS; i++) {
        if (umgr->app_list[i].handler == 0) {
            break;
        }
    }
    if (i >= MAX_UCODE_APPS) {
        UCODEMGR_UNLOCK(unit);
        return SOC_E_FULL;
    }
    sal_snprintf(umgr->app_list[i].type, 16, type);
    umgr->app_list[i].handler = func;
    umgr->app_list[i].asminit = pkgf;
  
    UCODEMGR_UNLOCK(unit);
    return rv;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_unregister_app
 * Description:
 *    Routine to register a callback. This can be used to override defaults
 */
int
soc_sbx_caladan3_ucodemgr_unregister_app(int unit, APP_INIT_F func)
{
    int i;
    soc_sbx_caladan3_ucodemgr_t *umgr;
    int rv = SOC_E_NONE;

    umgr = &_ucodemgr[unit];
    UCODEMGR_LOCK(unit);

    for (i=0; i < MAX_UCODE_APPS; i++) {
        if (umgr->app_list[i].handler == func) {
            break;
        }
    }
    if (i >= MAX_UCODE_APPS) {
        UCODEMGR_UNLOCK(unit);
        return SOC_E_NOT_FOUND;
    }
    sal_memset(umgr->app_list[i].type, 0, 16);
    umgr->app_list[i].handler = NULL;
    umgr->app_list[i].asminit = NULL;
  
    UCODEMGR_UNLOCK(unit);
    return rv;
}


/*
 * Function: 
 *    soc_sbx_caladan3_lr_ucodemgr_loadimg
 * Description:
 *    Routine initializes the asm interfaces and starts downloading
 *    image to the LRP instruction memory. Calls back application
 *    init routine at the end of the process
 */
int 
soc_sbx_caladan3_ucodemgr_loadimg(int unit)
{
    soc_sbx_caladan3_ucodemgr_t *umgr;
    int rv = SOC_E_NONE;

#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 regval = 0;
#endif

    UCODEMGR_LOCK(unit);
    umgr = &_ucodemgr[unit];

    if (UCODEMGR_CHECKALL(umgr)) {
        umgr->reload = 0;
        if (!umgr->ucode) {
            umgr->ucode = sal_alloc(sizeof(soc_sbx_caladan3_ucode_pkg_t),
                                    "Ucode package");
            if (umgr->ucode == NULL) {
                 UCODEMGR_UNLOCK(unit);
                return SOC_E_MEMORY;
            }
        }
        sal_memset(umgr->ucode, 0, sizeof(soc_sbx_caladan3_ucode_pkg_t));

        /* Init Asm interfaces */
        rv = umgr->asm_init(unit, umgr->ucode);
        if (SOC_SUCCESS(rv)) {
            /* Prepare LR for download */
            rv = umgr->lr_prepare(unit, umgr->ucode);
            if (SOC_SUCCESS(rv)) {
                /* Download  image */
                rv = umgr->lr_download(unit, umgr->ucode);

#ifdef BCM_WARM_BOOT_SUPPORT
		if(SOC_WARM_BOOT(unit)) {
		    /* set standby bank to opposite of active bank */
		    READ_LRA_BANK_ACTIVEr( unit, &regval );         
		    if(soc_reg_field_get( unit, LRA_BANK_ACTIVEr, regval, ACTIVEf) > 0) {
		        umgr->bank = 0;
		    }
		    else {
		        umgr->bank = 1;
		    }			
		}
#endif
		
                if (SOC_SUCCESS(rv)) {
                   /* Notify Application to initialize */
                   rv = umgr->app_init(unit, umgr->ucode);
                   if (SOC_SUCCESS(rv)) {
                       /* Notify LR download completed */
                       rv = umgr->lr_done(unit, umgr->ucode);
                       if (SOC_FAILURE(rv)) {
                           LOG_ERROR(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "soc_sbx_caladan3_ucodemgr_loadimg: unit %d Lr Done failed \n"), unit));
                       }
                   } else {
                       LOG_ERROR(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "soc_sbx_caladan3_ucodemgr_loadimg: unit %d App init failed \n"), unit));
                   }
               } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_caladan3_ucodemgr_loadimg: unit %d Lr Download Image failed \n"), unit));
               }
           } else {
               LOG_ERROR(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "soc_sbx_caladan3_ucodemgr_loadimg: unit %d Lr Prepare failed \n"), unit));
           }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_loadimg: unit %d Asm interface init failed \n"), unit));
        }

        /* return load status */ 
         UCODEMGR_UNLOCK(unit);
        return rv;
    }

    UCODEMGR_UNLOCK(unit);


    /* Not all interfaces are enabled */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "soc_sbx_caladan3_ucodemgr_loadimg: unit %d download interfaces not ready \n"), unit));

    return SOC_E_INIT;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_loadimg_from_pkg
 * Description:
 *    Routine to (re)load a microcode image from a new package file
 */
int 
soc_sbx_caladan3_ucodemgr_loadimg_from_pkg(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg, int reset, int force)
{
    soc_sbx_caladan3_ucodemgr_t *umgr;
    int rv = SOC_E_NONE;

    umgr = &_ucodemgr[unit];

    if (UCODEMGR_CHECKALL(umgr)) {
        umgr->reload = 1;

        /* Temp pkg does not include r/w methods so add them */
        C3Asm3__PkgInt__initF(pkg, umgr->ucode->m_fPut, umgr->ucode->m_fGet, umgr->ucode->m_pv);

        /* Save current symbol table & values */
        if (0 == force) {
            rv = C3Asm3__PkgInt__backupNc(umgr->ucode, pkg);
        } else {
        	rv = C3Asm3__PkgInt__backupNc__force(umgr->ucode, pkg);
        }
        if (rv != 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                                  " unit %d Nc backup failed \n"), unit));
            return rv;
        }
        /* Prepare LR for download */
        rv = umgr->lr_prepare(unit, pkg);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                                  "unit %d Lr Prepare failed \n"), unit));
            return rv;
        }
        /* Download  image */
        rv = umgr->lr_download(unit, pkg);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                                  " unit %d Lr Download Image failed \n"), unit));
            return rv;
        }
        /* Notify Application to initialize */
        rv = umgr->app_init(unit, pkg);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                                  " unit %d App init failed \n"), unit));
            return rv;
        }
        /* Notify LR download completed */
        rv = umgr->lr_done(unit, pkg);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                                  " unit %d Lr Done failed \n"), unit));
            return rv;
        }

        /* If not resetting then restore global values */
        if (reset == 0) {
        	if (0 == force) {
                rv = C3Asm3__PkgInt__restoreNc(unit, pkg);
        	} else {
                rv = C3Asm3__PkgInt__restoreNc__force(unit, pkg);
        	}
            if (rv != 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                                      " unit %d Nc restore failed \n"), unit));
                return rv;
            }
        }

        /* return load status */
        return rv;
    }

    /* Not all interfaces are enabled */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "soc_sbx_caladan3_ucodemgr_loadimg_from_pkg:"
                          " unit %d download interfaces not ready \n"), unit));

    return SOC_E_INIT;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_bank_swapped
 * Description:
 *    Adjust the standby bank and update dirty
 */
int 
soc_sbx_caladan3_ucodemgr_bank_swapped(int unit)
{
    int bank = _ucodemgr[unit].bank;
    _ucodemgr[unit].dirty[bank] = BANK_CLEAN;
    _ucodemgr[unit].bank = (bank+1) % SOC_SBX_CALADAN3_LR_INST_NUM_BANKS;
    return SOC_E_NONE;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_standby_bank_get
 * Description:
 *    Get the standby bank
 */
int 
soc_sbx_caladan3_ucodemgr_standby_bank_get(int unit)
{
    return _ucodemgr[unit].bank;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_standby_bank_set
 * Description:
 *    set the standby bank
 */
int 
soc_sbx_caladan3_ucodemgr_standby_bank_set(int unit, int bank)
{
    _ucodemgr[unit].bank = bank;
    return SOC_E_NONE;
}
/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_sync_all_banks
 * Description:
 *    return the current state of sync flag
 */
int 
soc_sbx_caladan3_ucodemgr_sync_all_banks(int unit)
{
    return _ucodemgr[unit].allbanks;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_bank_dirty_set
 * Description:
 *    return the current state of sync flag
 */
int 
soc_sbx_caladan3_ucodemgr_bank_dirty_set(int unit, int bank, int dirty)
{
    return _ucodemgr[unit].dirty[bank] = dirty;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_bank_dirty_get
 * Description:
 *    return the current state of sync flag
 */
int 
soc_sbx_caladan3_ucodemgr_bank_dirty_get(int unit, int bank)
{
    return _ucodemgr[unit].dirty[bank];
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_sym_get
 * Description:
 *    get the current value of a ucode symbol
 */
int 
soc_sbx_caladan3_ucodemgr_sym_get(int unit, 
                                  soc_sbx_caladan3_ucode_pkg_t *ucode,
                                  char *nc, uint32 *val)
{
    int rv;
    
#if 0
      char *p, buffer[128];
    int size = 0, skipsize = 0;
#endif     

    if (!nc || !val) {
      return SOC_E_PARAM;
    }
#if 0

    /*
     * C3 Simulator doesnt implement the entire NCAT table, So we will limit all
     * get only to the NCAT table of the c3 package
     */
    if (SAL_BOOT_BCMSIM) {
        sal_memset(buffer, 0, sizeof(buffer));
        size = soc_sbx_caladan3_sim_block_encode_simple(buffer, "global", nc, "get");
        rv = soc_sbx_caladan3_sim_sendrcv(unit, buffer, &size);
        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_sim_status_decode(buffer, "status", &skipsize);
        }
        if (SOC_SUCCESS(rv)) {
            p = buffer + skipsize;
            *val = soc_sbx_caladan3_sim_verb_decode(p, nc, &skipsize);
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "sim get failed (%d)\n"), rv));
            return rv;
        }
    } else
#endif    
    {
        rv = C3Asm3__PkgInt__getNc(ucode, nc, val);
    }
    if (rv != 0) {
        if (rv == 6) {

#ifdef BCM_WARM_BOOT_SUPPORT
	    if(SOC_WARM_BOOT(unit)) {
	        /* Modified push down values are expected, don't print message during warm boot */
	        return SOC_E_NONE;
	    }
#endif /* BCM_WARM_BOOT_SUPPORT */

            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Modified value of %s has been overwritten with a default value from ucode\n"), nc));
            return SOC_E_NONE;
        } else {
            /* Need a mapping here */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_sym_get: Failed getting value for %s rv(%d) \n"), nc, rv));
        }
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_sym_set
 * Description:
 *    set a ucode symbol to the given value
 */
int 
soc_sbx_caladan3_ucodemgr_sym_set(int unit, 
                                  soc_sbx_caladan3_ucode_pkg_t *ucode,
                                  char *nc, uint32 val)
{
    int rv;
    char *p, buffer[128];
    int size = 0, skipsize = 0;

    if (!nc || !ucode) {
        return SOC_E_PARAM;
    }
    if (SAL_BOOT_BCMSIM) {
        sal_memset(buffer, 0, sizeof(buffer));
        size = soc_sbx_caladan3_sim_block_encode_simple(buffer, "global", "", "set");
	if (size < 0) {
	    return SOC_E_PARAM;
	}
        p = buffer + size;
        size += soc_sbx_caladan3_sim_verb_encode(p, nc, val); 
        rv = soc_sbx_caladan3_sim_sendrcv(unit, buffer, &size);
        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_sim_status_decode(buffer, "status", &skipsize);
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "sim set failed (%d)\n"), rv));
            return rv;
        }
    } else {
        sal_mutex_take(bankSwapLock[unit], sal_mutex_FOREVER);

        rv = C3Asm3__PkgInt__putNc(ucode, nc, &val);

        sal_mutex_give(bankSwapLock[unit]);
    }
    if(rv != 0) {
        /* Need a mapping here */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucodemgr_sym_set: Failed setting value for %s rv(%d) \n"), nc, rv));
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 * Function: 
 *    soc_sbx_caladan3_ucodemgr_print_state
 * Description:
 *    displays internal state
 */
void
soc_sbx_caladan3_ucodemgr_print_state(int unit)
{
    soc_sbx_caladan3_ucodemgr_t *ucodemgr;
    int i;
    ucodemgr = &_ucodemgr[unit];

    LOG_CLI((BSL_META_U(unit,
                        "ucodemgr->bank      : %d\n"),ucodemgr->bank)); 
    LOG_CLI((BSL_META_U(unit,
                        "ucodemgr->allbanks  : %d\n"),ucodemgr->allbanks));
    for(i=0; i<SOC_SBX_CALADAN3_LR_INST_NUM_BANKS; i++) {
        LOG_CLI((BSL_META_U(unit,
                            "ucodemgr->dirty[%d]  : %d\n"),i, ucodemgr->dirty[i]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "ucodemgr->reload    : %d\n"),ucodemgr->reload));

} 


#endif /* BCM_CALADAN3_SUPPORT */
