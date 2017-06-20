/*
 * $Id: sbx_common.c,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * configuration common to sbx devices
 *
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sirius.h>
#include <soc/debug.h>
#include "bm9600_properties.h"
#include <bcm/cosq.h>


static int soc_sbx_conn_cmp(void *src, void *dest);
static void soc_sbx_conn_cpy(void *a, void *b);
static void soc_sbx_conn_init(void *a);

static sbx_conn_template_info_t *conn_util[SOC_MAX_NUM_DEVICES];
static sbx_conn_template_info_t *conn_age[SOC_MAX_NUM_DEVICES];

static sbx_template_clbk_t template_clbk[1] =
{
    { soc_sbx_conn_cmp, soc_sbx_conn_cpy, soc_sbx_conn_cpy, soc_sbx_conn_init },
};

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0

int
soc_sbx_wb_connect_state_init(int unit)
{
    int                 rv = SOC_E_NONE;
    soc_scache_handle_t scache_handle;
    uint8               *scache_ptr = NULL, *ptr, *end_ptr;
    uint32              scache_len, cu_len, ca_len;
    int                 stable_size;
    uint16               default_ver = BCM_WB_DEFAULT_VERSION;
    uint16               recovered_ver = BCM_WB_DEFAULT_VERSION;

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, SOC_SBX_WB_MODULE_CONNECT, 0);

    if (SOC_WARM_BOOT(unit)) {
        rv = soc_versioned_scache_ptr_get(unit, scache_handle, FALSE,
					  &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            return rv;
        }
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len; /* used for overrun checks*/

    /* If device is during warm-boot, recover the state from scache */
    /* Otherwise, calculate the maximum storage requirements        */
    /* 
     * Compression Format:
     *      0-3:    conn_util_max_template
     *      4-7:    conn_age_max_template
     *      7-A:    A = 7 + (conn_util_max_template * sizeof(sbx_conn_template_t))
     *      A-B:    Y = X + (conn_age_max_template * sizeof(sbx_conn_template_t))
     */
    __WB_DECOMPRESS_SCALAR(uint32, conn_util[unit]->max_template);
    __WB_DECOMPRESS_SCALAR(uint32, conn_age[unit]->max_template);
    
    cu_len = sizeof(sbx_conn_template_t) * conn_util[unit]->max_template;
    if (SOC_WARM_BOOT(unit)) {
        sal_memcpy(conn_util[unit]->template, ptr, cu_len);
        ptr += cu_len;
    } else {
        scache_len += cu_len;
    }

    ca_len = sizeof(sbx_conn_template_t) * conn_age[unit]->max_template;
    if (SOC_WARM_BOOT(unit)) {
        sal_memcpy(conn_age[unit]->template, ptr, ca_len); 
        ptr += ca_len;
    } else {
        scache_len += ca_len;
    }

    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    if (!SOC_WARM_BOOT(unit)) {
        rv = soc_versioned_scache_ptr_get(unit, scache_handle,TRUE,
					  &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            return rv;
        }
    }

    return rv;
}

int
soc_sbx_wb_connect_state_sync(int unit, int sync)
{
    int                 rv = SOC_E_NONE;
    soc_scache_handle_t scache_handle;
    uint8               *scache_ptr = NULL, *ptr, *end_ptr;
    uint32              scache_len = 0, cu_len, ca_len;
    int                 stable_size;
    uint16              default_ver = BCM_WB_DEFAULT_VERSION;
    uint16              recovered_ver = BCM_WB_DEFAULT_VERSION;

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    if (SOC_WARM_BOOT(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Cannot write to SCACHE during WarmBoot\n")));
        return SOC_E_INTERNAL;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, SOC_SBX_WB_MODULE_CONNECT, 0);

    rv = soc_versioned_scache_ptr_get(unit, scache_handle, FALSE,
                                      &scache_len, &scache_ptr,
                                      default_ver, &recovered_ver);
    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        return rv;
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len; /* used for overrun checks*/

    /* now store the state into the compressed format */
    /* 
     * Compression Format:
     *      0-3:    conn_util_max_template
     *      4-7:    conn_age_max_template
     *      7-A:    A = 7 + (conn_util_max_template * sizeof(sbx_conn_template_t))
     *      A-B:    Y = X + (conn_age_max_template * sizeof(sbx_conn_template_t))
     */
    __WB_COMPRESS_SCALAR(uint32, conn_util[unit]->max_template);
    __WB_COMPRESS_SCALAR(uint32, conn_age[unit]->max_template);
    
    cu_len = sizeof(sbx_conn_template_t) * conn_util[unit]->max_template;
    sal_memcpy(ptr, conn_util[unit]->template, cu_len);
    ptr += cu_len;

    ca_len = sizeof(sbx_conn_template_t) * conn_age[unit]->max_template;

    sal_memcpy(ptr, conn_age[unit]->template, ca_len);
    ptr += ca_len;

    /* trigger the scache sync */
    if (sync) {
        rv = soc_scache_commit(unit);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) sync'ing scache to "
                                  "Persistent memory. \n"),FUNCTION_NAME(), soc_errmsg(rv)));
            return rv;
        }
    }

    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    return rv;
}

#endif /* BCM_WARM_BOOT_SUPPORT */

static int
soc_sbx_conn_cmp(void *a, void *b)
{
    return ( ( ((*(uint32 *)a) == (*(uint32 *)b)) ? TRUE : FALSE ) );
}

static void
soc_sbx_conn_cpy(void *src, void *dest)
{
    (*(uint32 *)dest) = (*(uint32 *)src);
}

static void
soc_sbx_conn_init(void *a)
{
    (*(uint32 *)a) = 0;
}

static int
soc_sbx_connect_template_get(int unit, sbx_conn_template_info_t *info,
                             void *value_p, int *template, sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if ( (conn[cos].in_use == TRUE) && (clbk->compare(&conn[cos].value, value_p)) ) {
            (*template) = conn[cos].template;
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

    return(rc);
}

static int
soc_sbx_connect_get(int unit, sbx_conn_template_info_t *info, int template, int *value_p,
                                                              sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if ( (conn[cos].in_use == TRUE) && (conn[cos].template == template) ) {
            clbk->get(&conn[cos].value, value_p);
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

    return(rc);
}

static int
soc_sbx_connect_alloc(int unit, sbx_conn_template_info_t *info, int flags,
                        void *value_p, int *is_allocated, int *template, sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos, cos_available = -1;
    sbx_conn_template_t *conn = info->template;

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (SOC_IS_RELOADING(unit)) {
        /*
         *  During easy reload, we pretend to allocate, but use the cache that
         *  was read during initialisation.
         */
        for (cos = 0; cos < info->max_template; cos++) {
            if (clbk->compare(&conn[cos].value, value_p)) {
                break;
            }
        }
        if (cos >= info->max_template) {
            /* config change attempted during reload */
            rc = BCM_E_CONFIG;
        }
        if (!conn[cos].in_use) {
            /* not marked as in-use yet, act like just allocated */
            cos_available = cos;
            cos = info->max_template;
        }
    } else { /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        for (cos = 0; cos < info->max_template; cos++) {
            if ( (conn[cos].in_use == TRUE) && (clbk->compare(&conn[cos].value, value_p)) ) {
                break;
            }

            if ( (cos_available == -1) && (conn[cos].in_use == FALSE) )  {
                cos_available = cos;
            }
        }
        if ( (cos >= info->max_template) && (cos_available == -1) ) {
            rc = SOC_E_RESOURCE;
            return(rc);
        }
#if defined(BCM_EASY_RELOAD_SUPPORT)
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    if (cos < info->max_template) {
        if (!(conn[cos].flags & SOC_SBX_CONN_FIXED)) {
            conn[cos].ref_cnt++;
        }
        (*template) = cos;
        (*is_allocated) = FALSE;
    } else {
        conn[cos_available].flags = flags;
        conn[cos_available].in_use = TRUE;
        conn[cos_available].template = cos_available;
        clbk->update(value_p, &conn[cos_available].value);
        conn[cos_available].ref_cnt = 1;

        (*template) = cos_available;
        (*is_allocated) = TRUE;
    }

    return(rc);
}

static int
soc_sbx_connect_dealloc(int unit, sbx_conn_template_info_t *info, int flags,
                        void *value_p, int *is_deallocated, int *template, sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if ( (conn[cos].in_use == TRUE) && (clbk->compare(&conn[cos].value, value_p)) ) {
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

   
    (*is_deallocated) = FALSE;
    (*template) = cos;
    if (!(conn[cos].flags & SOC_SBX_CONN_FIXED)) {
        conn[cos].ref_cnt--;
        if (conn[cos].ref_cnt == 0) {
            conn[cos].in_use = FALSE;
            conn[cos].template = -1;
            clbk->init(&conn[cos].value);
            conn[cos].flags = 0;
 
            (*is_deallocated) = TRUE;
        }
    }

    return(rc);
}

int
soc_sbx_connect_deinit(int unit)
{
    if (conn_util[unit]) {
        if (conn_util[unit]->template) {
            sal_free(conn_util[unit]->template);
            conn_util[unit]->template = NULL;
        }
        sal_free(conn_util[unit]);
        conn_util[unit] = NULL;
    }
    if (conn_age[unit]) {
        if (conn_age[unit]->template) {
            sal_free(conn_age[unit]->template);
            conn_age[unit]->template = NULL;
        }
        sal_free(conn_age[unit]);
        conn_age[unit] = NULL;
    }
    return BCM_E_NONE;
}


/*
 *  Note that templates are tracked at hardware granularity, so if the value is
 *  represented at BCM layer as a percentage, and the hardware only has eight
 *  possible settings, there will only be eight distinct values in the
 *  templates, and in the cache.
 */
int
soc_sbx_connect_init(int unit, int conn_util_max, int conn_age_max)
{
    int rc = SOC_E_NONE;
    int cos;

    SOC_IF_ERROR_RETURN(soc_sbx_connect_deinit(unit));

    conn_util[unit] = sal_alloc(sizeof(sbx_conn_template_info_t), "conn_util");
    if (conn_util[unit] == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }
    sal_memset(conn_util[unit], 0, sizeof(sbx_conn_template_info_t));

    conn_age[unit] = sal_alloc(sizeof(sbx_conn_template_info_t), "conn_age");
    if (conn_age[unit] == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }
    sal_memset(conn_age[unit], 0, sizeof(sbx_conn_template_info_t));

    conn_util[unit]->template =
                     sal_alloc(sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS, "conn_template");
    if (conn_util[unit]->template == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }

    conn_age[unit]->template =
                      sal_alloc(sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS, "conn_age");
    if (conn_age[unit]->template == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(conn_util[unit]->template, 0, (sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS));
    sal_memset(conn_age[unit]->template, 0, (sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS));
    conn_util[unit]->max_template = conn_util_max;
    conn_age[unit]->max_template = conn_age_max;
    for (cos = 0; cos < conn_util[unit]->max_template; cos++) {
        conn_util[unit]->template[cos].in_use = FALSE;
        conn_util[unit]->template[cos].template = -1;
    }
    for (cos = 0; cos < conn_age[unit]->max_template; cos++) {
        conn_age[unit]->template[cos].in_use = FALSE;
        conn_age[unit]->template[cos].template = -1;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    rc = soc_sbx_wb_connect_state_init(unit);
#endif
    return(rc);

err:
    soc_sbx_wb_connect_state_deinit(unit);
    soc_sbx_connect_deinit(unit);
    return(rc);
}

/*
 *  Reload preparation -- puts the proper values into the cache, so that during
 *  reload we can guess the proper template to 'allocate'.
 */
int
soc_sbx_connect_reload(int unit, uint32 *util, uint32 *age)
{
#if defined(BCM_EASY_RELOAD_SUPPORT)
    unsigned int index;

    for (index = 0; index < conn_util[unit]->max_template; index++) {
        template_clbk[0].update(&(util[index]),&(conn_util[unit]->template[index].value));
        
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "set conn_util[%d]->template[%d] to %08X (%d)\n"),
                  unit,
                  index,
                  util[index],
                  util[index]));
    }
    for (index = 0; index < conn_age[unit]->max_template; index++) {
        template_clbk[0].update(&(age[index]),&(conn_age[unit]->template[index].value));
        
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "set conn_age[%d]->template[%d] to %08X (%d)\n"),
                  unit,
                  index,
                  age[index],
                  age[index]));
    }
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
    return SOC_E_NONE;
}

int
soc_sbx_wb_connect_state_deinit(int unit)
{
    int rc = SOC_E_NONE;


    if (conn_util[unit] != NULL) {
        if (conn_util[unit]->template != NULL) {
            sal_free(conn_util[unit]->template);
        }
        sal_free(conn_util[unit]);
    }
    if (conn_age[unit] != NULL) {
        if (conn_age[unit]->template != NULL) {
            sal_free(conn_age[unit]->template);
        }
        sal_free(conn_age[unit]);
    }

    return(rc);
}

int
soc_sbx_connect_min_util_template_get(int unit, int utilization, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_template_get(unit, conn_util[unit], (void *)&utilization,
                                                            template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_min_util_get(int unit, int template, int *utilization)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_get(unit, conn_util[unit], template, (void *)utilization, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_min_util_alloc(int unit, int flags, int utilization, int *is_allocated,
                                                                         int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_alloc(unit, conn_util[unit], flags, (void *)&utilization, is_allocated,
                                                             template, &template_clbk[0]);

#if defined(BROADCOM_DEBUG)
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "alloc conn_util[%d]->template[%d] to %08X (%d)\n"),
              unit,
              *template,
              utilization,
              utilization));
#endif /* BROADCOM_DEBUG */

    return(rc);
}

int
soc_sbx_connect_min_util_dealloc(int unit, int flags, int utilization, int *is_deallocated,
                                                                                int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_dealloc(unit, conn_util[unit], flags, (void *)&utilization, is_deallocated,
                                                              template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_max_age_template_get(int unit, int age, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_template_get(unit, conn_age[unit], (void *)&age,
                                                            template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_max_age_get(int unit, int template, int *age)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_get(unit, conn_age[unit], template, (void *)age, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_max_age_alloc(int unit, int flags, int age, int *is_allocated, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_alloc(unit, conn_age[unit], flags, (void *)&age, is_allocated, template,
                                                                             &template_clbk[0]);

#if defined(BROADCOM_DEBUG)
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "alloc conn_age[%d]->template[%d] to %08X (%d)\n"),
              unit,
              *template,
              age,
              age));
#endif /* BROADCOM_DEBUG */
    return(rc);
}

int
soc_sbx_connect_max_age_dealloc(int unit, int flags, int age, int *is_deallocated, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_dealloc(unit, conn_age[unit], flags, (void *)&age, is_deallocated,
                                                                   template, &template_clbk[0]);
    return(rc);
}


int
soc_sbx_sched_get_internal_state(int unit, int sched_mode, int int_pri, int *queue_type, int *priority, int *priority2)
{
    int rc = SOC_E_NONE;


    /* determine the template that is being used */
    /* NOTE: There should be an extra SOC property for the template. */

    (*priority2) = -1;
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                (*priority2) = 3; /* Satisfied */
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 14;
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                (*priority2) = 3; /* Satisfied */
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0; 
                (*priority) = 2;
                break;

            case BCM_COSQ_SP:
            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                if ((int_pri >= 0) && (int_pri <= 3)) {
                    (*priority) = 12 - int_pri;
                }
                else { /* ((int_pri >= 4) && (int_pri <= 7)) */
                    (*priority) = 7 - (int_pri - 4);
                }
                break;
            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "scheduler mode(%d) unknown\n"), sched_mode));
                return(SOC_E_PARAM);
        }
    }
    else {
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 11; /* Hungry */
                (*priority2) = 2; /* Satisfied */
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 13;
                (*priority2) = 14; /* Satisfied */
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 11; /* Hungry */
                (*priority2) = 2; /* Satisfied */
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0;
                (*priority) = 2;
                break;

            case BCM_COSQ_SP:
                (*queue_type) = 1;
                (*priority) = 11; /* Hungry */
                (*priority2) = 2; /* Satisfied */
                break;

            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                (*priority) = 10 - int_pri;
                break;

            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "scheduler mode(%d) unknown\n"), sched_mode));
                return(SOC_E_PARAM);
        }
    }

    return(rc);
}
/* Similar to previous except supports BCM_COSQ_SP0-7 and int_pri unused for those */
int
soc_sbx_sched_get_internal_state_queue_attach(int unit, int sched_mode, int int_pri,
					      int *queue_type, int *priority, int *hungry_priority)
{
    int rc = SOC_E_NONE;


    /* determine the template that is being used */
    /* NOTE: There should be an extra SOC property for the template. */
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        *hungry_priority = 8;
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 14;
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0; 
                (*priority) = 2;
                break;

            case BCM_COSQ_SP7:
            case BCM_COSQ_GSP7:
                (*queue_type) = 8;
		(*priority) = 12;
                break;

            case BCM_COSQ_SP6:
            case BCM_COSQ_GSP6:
                (*queue_type) = 9;
		(*priority) = 11;
                break;

            case BCM_COSQ_SP5:
            case BCM_COSQ_GSP5:
                (*queue_type) = 10;
		(*priority) = 10;
                break;

            case BCM_COSQ_SP4:
            case BCM_COSQ_GSP4:
                (*queue_type) = 11;
		(*priority) = 9;
                break;

            case BCM_COSQ_SP3:
            case BCM_COSQ_GSP3:
                (*queue_type) = 12;
		(*priority) = 7;
                break;

            case BCM_COSQ_SP2:
            case BCM_COSQ_GSP2:
                (*queue_type) = 13;
		(*priority) = 6;
                break;

	    case BCM_COSQ_SP1:
            case BCM_COSQ_GSP1:
                (*queue_type) = 14;
		(*priority) = 5;
                break;

            case BCM_COSQ_SP0:
            case BCM_COSQ_GSP0:
                (*queue_type) = 15;
		(*priority) = 4;
                break;

            case BCM_COSQ_SP:
            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                if ((int_pri >= 0) && (int_pri <= 3)) {
                    (*priority) = 12 - int_pri;
                }
                else { /* ((int_pri >= 4) && (int_pri <= 7)) */
                    (*priority) = 7 - (int_pri - 4);
                }
                break;

            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "scheduler mode(%d) unknown\n"), sched_mode));
                return(SOC_E_PARAM);
        }
    }
    else {
	*hungry_priority = 11;
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 11;
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 13;
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 11;
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0;
                (*priority) = 2;
                break;

            case BCM_COSQ_SP:
            case BCM_COSQ_SP7:
            case BCM_COSQ_SP6:
            case BCM_COSQ_SP5:
            case BCM_COSQ_SP4:
            case BCM_COSQ_SP3:
            case BCM_COSQ_SP2:
            case BCM_COSQ_SP1:
            case BCM_COSQ_SP0:
                (*queue_type) = 1;
                (*priority) = 11;
                break;

            case BCM_COSQ_GSP7:
                (*queue_type) = 8;
		(*priority) = 10;
                break;

            case BCM_COSQ_GSP6:
                (*queue_type) = 9;
		(*priority) = 9;
                break;

            case BCM_COSQ_GSP5:
                (*queue_type) = 10;
		(*priority) = 8;
                break;

            case BCM_COSQ_GSP4:
                (*queue_type) = 11;
		(*priority) = 7;
                break;

            case BCM_COSQ_GSP3:
                (*queue_type) = 12;
		(*priority) = 6;
                break;

            case BCM_COSQ_GSP2:
                (*queue_type) = 13;
		(*priority) = 5;
                break;

            case BCM_COSQ_GSP1:
                (*queue_type) = 14;
		(*priority) = 4;
                break;

            case BCM_COSQ_GSP0:
                (*queue_type) = 15;
		(*priority) = 3;
                break;

            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                (*priority) = 10 - int_pri;
                break;

            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "scheduler mode(%d) unknown\n"), sched_mode));
                return(SOC_E_PARAM);
        }
    }

    return(rc);
}


int
soc_sbx_sched_config_params_verify(int unit, int sched_mode, int int_pri)
{
    int rc = SOC_E_NONE;


    /* determine the template that is being used */
        if ( ((sched_mode == BCM_COSQ_SP_GLOBAL) || (sched_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG))
                                                        && !((int_pri >= 0) && (int_pri <= 7)) ) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "int_pri(%d) not supported\n"), int_pri));
            return(SOC_E_PARAM);
    }

    return(rc);
}

int
soc_sbx_sched_config_set_params_verify(int unit, int sched_mode, int int_pri)
{
    int rc = SOC_E_NONE;


    switch (sched_mode) {
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
        case BCM_COSQ_EF:
        case BCM_COSQ_AF:
        case BCM_COSQ_BE:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "mode(%d) not supported for fifo re-direction\n"), sched_mode));
            return(SOC_E_PARAM);

        case BCM_COSQ_SP:
        case BCM_COSQ_SP_GLOBAL:
            break;

        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "mode(%d) unknown for fifo re-direction\n"), sched_mode));
            return(SOC_E_PARAM);
    }

    /* determine the template that is being used */
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if ( ((sched_mode == BCM_COSQ_SP_GLOBAL) || (sched_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG))
                                                        && !((int_pri >= 0) && (int_pri <= 7)) ) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "int_pri(%d) not supported\n"), int_pri));
            return(SOC_E_PARAM);
        }
    }
    else {
        if (sched_mode == BCM_COSQ_SP) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "mode(%d) not supported for fifo re-direction\n"), sched_mode));
            return(SOC_E_PARAM);
        }

        if ( (sched_mode == BCM_COSQ_SP_GLOBAL) && !((int_pri >= 0) && (int_pri <= 7)) ) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "int_pri(%d) not supported\n"), int_pri));
            return(SOC_E_PARAM);
        }
    }

    return(rc);
}

