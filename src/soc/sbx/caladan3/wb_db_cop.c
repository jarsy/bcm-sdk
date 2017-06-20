/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/types.h>
#include <shared/bitop.h>
#include <shared/util.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <soc/sbx/wb_db_cmn.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/caladan3/wb_db_cop.h>
#include <soc/sbx/sbx_drv.h>


#ifdef BCM_WARM_BOOT_SUPPORT
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif /* _ERR_MSG_MODULE_NAME */

/* there isn't a soc module specific for g3p1. init is as close as it gets */
#define _ERR_MSG_MODULE_NAME BSL_LS_SOC_INIT


/* global data structures */
/*------------------------*/
static _soc_sbx_cop_wb_state_scache_info_t *_soc_sbx_cop_wb_state_scache_info_p[SOC_MAX_NUM_DEVICES] = { 0 };


/* macros */
/*--------*/
#define SBX_SCACHE_INFO_PTR(unit) (_soc_sbx_cop_wb_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_soc_sbx_cop_wb_state_scache_info_p[unit]) != NULL \
        && (_soc_sbx_cop_wb_state_scache_info_p[unit]->init_done == TRUE))

#define COP_PROFILE_SIZE (sizeof(soc_sbx_caladan3_cop_profile_t) * \
                          SOC_SBX_CALADAN3_COP_NUM_PROFILE)

#define COP_TOTAL_SIZE (COP_PROFILE_SIZE*SOC_SBX_CALADAN3_COP_NUM_COP)

#define COP_PROFILE_GET(unit,cop) (SOC_SBX_CFG_CALADAN3(unit)->cop_cfg.profiles[cop])


/* static functions */
/*------------------*/



/*
 *  Function
 *     _soc_sbx_cop_wb_dump_state
 *  Description:
 *     CRC the contents of the COP buffer and display on the console.
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

int 
_soc_sbx_cop_wb_dump_state(int unit, int copNum)
{
    int i;
    unsigned int crc_cop;
    
    if (copNum == -1) {
        for (i=0; i<SOC_SBX_CALADAN3_COP_NUM_COP; i++) {
            crc_cop = _shr_crc32(0, (unsigned char *)COP_PROFILE_GET(unit, i), 
                                 COP_PROFILE_SIZE);
            
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s unit %d, soc COP warm boot scache signatures cop%d profile 0x%08x.\n"),
                         FUNCTION_NAME(), unit, i, crc_cop));
        }
        return 0;
    } else if ((copNum >= 0) && (copNum < SOC_SBX_CALADAN3_COP_NUM_COP)) {
        crc_cop = _shr_crc32(0, (unsigned char *)COP_PROFILE_GET(unit, copNum), 
                             COP_PROFILE_SIZE);

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s unit %d, soc COP warm boot scache signatures cop%d profile 0x%08x.\n"),
                     FUNCTION_NAME(), unit, copNum, crc_cop));
        return crc_cop;
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s unit %d, Invalid cop number %d.\n"),
                   FUNCTION_NAME(), unit, copNum));
    }
    return 0;
}


/*
 *  Function
 *     _soc_sbx_cop_wb_state_scache_alloc
 *  Description:
 *     alloc scache for soc cop module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_cop_wb_state_scache_alloc(int unit)
{
    int rv = SOC_E_NONE;

    _soc_sbx_cop_wb_state_scache_info_p[unit] = sal_alloc(sizeof(_soc_sbx_cop_wb_state_scache_info_t), 
                                                          "Scache for COP Alloc warm boot");

    if (_soc_sbx_cop_wb_state_scache_info_p[unit] == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed to allocate scache for for soc soc COP, unit %d \n"),
                   unit));
        rv = SOC_E_MEMORY;
    }
    return rv;
}


/*
 *  Function
 *     _soc_sbx_cop_wb_state_scache_free
 *  Description:
 *     free scache for soc cop module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC void
_soc_sbx_cop_wb_state_scache_free(int unit)
{
    sal_free(_soc_sbx_cop_wb_state_scache_info_p[unit]);
}


/*
 *  Function
 *       _soc_sbx_cop_wb_alloc_layout_init
 *  Description:
 *       sets up the scache structure for soc cop module
 *  Inputs:
 *     unit - device number
 *     version - version id
 *  Outputs: 
 *     scache_len - length of scache in bytes
 */

STATIC int
_soc_sbx_cop_wb_alloc_layout_init(int unit, int version, 
                                  unsigned int *scache_len)
{
    int rc = SOC_E_NONE;
    _soc_sbx_cop_wb_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = _soc_sbx_cop_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                              unit));

        return SOC_E_INIT;
    }
    switch(version)
    {
    case SOC_CALADAN3_COP_WB_VERSION_1_0:
        wb_info_ptr->version = version;
        wb_info_ptr->scache_len = COP_TOTAL_SIZE;
        *scache_len = COP_TOTAL_SIZE;
        break;

    default:
        rc = SOC_E_INTERNAL;
        SOC_IF_ERROR_RETURN(rc);
        /* coverity [dead_error_line] */
        break;
    }
    return rc;
}


/*
 *  Function
 *     _soc_sbx_cop_wb_alloc_restore
 *  Description:
 *     Restore the soc cop data from scache
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_cop_wb_alloc_restore(int unit)
{
    int rv = SOC_E_NONE;
    int i;
    int size;
    _soc_sbx_cop_wb_state_scache_info_t *wb_info_ptr = NULL;

    wb_info_ptr = _soc_sbx_cop_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"), 
                              unit));
        return SOC_E_INIT;
    }

    switch(wb_info_ptr->version)
    {
    case SOC_CALADAN3_COP_WB_VERSION_1_0:
        size = 0;
        for (i=0; i<SOC_SBX_CALADAN3_COP_NUM_COP; i++) {
            SBX_WB_DB_RESTORE_MEMORY(uint8, COP_PROFILE_SIZE, COP_PROFILE_GET(unit, i));
            size += COP_PROFILE_SIZE;
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Restored soc COP from warm boot scache. "
                                "unit %d loaded %d bytes\n"), 
                     unit, size));

        _soc_sbx_cop_wb_dump_state(unit, -1);
        break;

    default:
        rv = SOC_E_INTERNAL;
        SOC_IF_ERROR_RETURN(rv);
        /* coverity[dead_error_line] */
        break;
    }
    return rv;
}


/* Externally accessible functions */
/*---------------------------------*/

/* compress and store soc cop info in warm boot scache */

extern int
soc_sbx_cop_wb_state_sync(int unit)
{
    int rv = SOC_E_NONE;
    int i;
    int size;
    uint8 *scache_ptr_orig = NULL;

    _soc_sbx_cop_wb_state_scache_info_t *wb_info_ptr = NULL;


    if((_soc_sbx_cop_wb_state_scache_info_p[unit] == NULL)
        || (_soc_sbx_cop_wb_state_scache_info_p[unit]->init_done != TRUE))
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc COP Warm boot scache not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    wb_info_ptr = _soc_sbx_cop_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    /* Save the original scache ptr, for allowing multiple syncs */
    scache_ptr_orig = wb_info_ptr->scache_ptr;

    switch(wb_info_ptr->version)
    {
        case SOC_CALADAN3_COP_WB_VERSION_1_0:
            size = 0;
            for (i=0; i<SOC_SBX_CALADAN3_COP_NUM_COP; i++) {
                SBX_WB_DB_SYNC_MEMORY(uint8, COP_PROFILE_SIZE, COP_PROFILE_GET(unit, i));
                size += COP_PROFILE_SIZE;
            }

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Saved soc COP to warm boot scache. "
                                    "unit %d saved %d bytes\n"), 
                         unit, size));

            _soc_sbx_cop_wb_dump_state(unit, -1);

            /* Restore the scache ptr to original */
            wb_info_ptr->scache_ptr = scache_ptr_orig;

            break;


        default:
            rv = SOC_E_INTERNAL;
            SOC_IF_ERROR_RETURN(rv);
            /* coverity [dead_error_line] */
            break;
    }
    return rv;
}


/* called from cop.c when executing an init during warm boot. */

extern int
soc_sbx_cop_wb_state_init(int unit)
{
    int rv = SOC_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    soc_scache_handle_t handle = 0;
    unsigned int scache_len = 0;
    uint8 *scache_ptr = NULL;
    uint16 version = SOC_CALADAN3_COP_WB_VERSION_CURR;
    uint16 recovered_version = 0;
    int exists = 0;
    _soc_sbx_cop_wb_state_scache_info_t *wb_info_ptr = NULL;

    if (SBX_SCACHE_INFO_PTR(unit))
    {
        _soc_sbx_cop_wb_state_scache_free(unit);
    }

    SOC_IF_ERROR_RETURN(_soc_sbx_cop_wb_state_scache_alloc(unit));
    wb_info_ptr = _soc_sbx_cop_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL)
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_COP, 0);

    if (SOC_WARM_BOOT(unit)) {
        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRetrieve, 
                                         flags, &scache_len, 
                                         &scache_ptr, version, 
                                         &recovered_version, &exists);
        if (rv == SOC_E_NONE)
        {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "unit %d loading COP Alloc backing store state\n"),
                         unit));

            wb_info_ptr->scache_ptr = scache_ptr;
            SOC_IF_ERROR_RETURN(_soc_sbx_cop_wb_alloc_layout_init(unit, 
                                                                  version, 
                                                                  &scache_len));
            if(scache_len != wb_info_ptr->scache_len)
            {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Scache length %d is not same as stored length %d\n"),
                                      scache_len, wb_info_ptr->scache_len));
            }

            SOC_IF_ERROR_RETURN(_soc_sbx_cop_wb_alloc_restore(unit));
            if (rv == SOC_E_NONE)
            {
                if (version != recovered_version)
                {
                    /* set up layout for the preferred version */
                    SOC_IF_ERROR_RETURN(_soc_sbx_cop_wb_alloc_layout_init(unit,
                                                                          version,
                                                                          &scache_len));
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "unit %d reallocate %d bytes warm" 
                                            " boot backing store space\n"), 
                                 unit, scache_len));

                    /* reallocate the warm boot space */
                    rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRealloc,
                                                         flags, &scache_len, &scache_ptr,
                                                         version, &recovered_version,
                                                         &exists);
                    if (rv != SOC_E_NONE)
                    {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "unable to reallocate %d bytes"
                                              "warm boot space for unit %d soc COP instance: %d (%s)\n"),
                                   scache_len, unit, rv, _SHR_ERRMSG(rv)));
                        return rv;
                    }
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == rv) */
            wb_info_ptr->scache_ptr = scache_ptr;
            _soc_sbx_cop_wb_state_scache_info_p[unit]->init_done = TRUE;

        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                  " unit %d soc COP instance: %d (%s)\n"),
                       unit, rv, _SHR_ERRMSG(rv)));
            return rv;
        }


    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        SOC_IF_ERROR_RETURN(_soc_sbx_cop_wb_alloc_layout_init(unit, version, 
                                                              &scache_len));
        
        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing" 
                                " store space\n"), unit, scache_len));

        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheCreate, 
                                             flags, &scache_len, &scache_ptr, 
                                             version, &recovered_version, &exists);
        if (rv != SOC_E_NONE)
        {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, rv, _SHR_ERRMSG(rv)));
            return rv;
        }
        else
        {
            wb_info_ptr->scache_ptr = scache_ptr;
            _soc_sbx_cop_wb_state_scache_info_p[unit]->init_done = TRUE;
        }
    }
    return rv;
}


void soc_sbx_cop_signature(int unit)
{
    int i;

    for (i=0; i<SOC_SBX_CALADAN3_COP_NUM_COP; i++) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc cop%d 0x%08x\n"),
                                i, _soc_sbx_cop_wb_dump_state(unit, i)));
    }
}

#endif /* BCM_WARM_BOOT_SUPPORT */

