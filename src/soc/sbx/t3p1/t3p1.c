/*
 * $Id: t3p1.c,v 1.1.2.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/sbx/t3p1/t3p1.h>
#include <soc/sbx/t3p1/t3p1_int.h>
#include <soc/sbx/t3p1/t3p1_defs.h>

#ifdef BCM_CALADAN3_T3P1_SUPPORT

#define PPE_CFG_INIT 1
#define PED_CFG_INIT 1
#define COP_CFG_INIT 1
#define CMU_CFG_INIT 1
#define TMU_CFG_INIT 1

/*
 * Function: t3p1_app_init
 *    Initialize the Empty Microcode Application
 * Parameters
 *    unit: Input, unit number
 *    contexts: Output, number of contexts used in application
 *    epoch: Output, epoch len or number of instr in application
 * Returns
 *    error status: one of SOC_E* 
 */
int
t3p1_app_init(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg, uint32 *contexts, uint32 *epoch, int reload)
{
    int rv = SOC_E_NONE;
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }

    if (!reload) {

        if (!sbx->drv) {
            sbx->drv = sal_alloc(sizeof(soc_sbx_t3p1_state_t), "t3p1 ucode state");
            if (!sbx->drv) {
                return SOC_E_MEMORY;
            }
        }
        sal_memset(sbx->drv, 0, sizeof(soc_sbx_t3p1_state_t));

#if COP_CFG_INIT
        rv = soc_sbx_t3p1_cop_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d COP init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif
#if CMU_CFG_INIT
        rv = soc_sbx_t3p1_cmu_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d CMU init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif

        rv = soc_sbx_t3p1_init(unit, (void*)pkg);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d microcode application init failed (%d) \n"), unit, rv));
            return rv;
        }

#if PED_CFG_INIT
        rv = soc_sbx_t3p1_ped_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d PED header init %d \n"), unit, rv));
            return rv;
        }
#endif
#if PPE_CFG_INIT
        rv = soc_sbx_t3p1_ppe_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d PPE configuration failed %d \n"), unit, rv));
            return SOC_E_PARAM;
        }
        rv = soc_sbx_t3p1_ppe_tables_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d PPE table manager init failed %d\n"),
                       unit, rv));
            return (rv);
        }
        /*
        rv = soc_sbx_t3p1_ppe_init_ext(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d PPE hand coded init failed %d\n"),
                       unit, rv));
            return (rv);
        }
        */
#endif
#if TMU_CFG_INIT
        rv = soc_sbx_t3p1_tmu_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d TMU table manager init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif

    }

    /*
     * Microcode application initialized successfully
     * Final steps before letting LR execute
     *   1) Set application params to LRP
     *   2) Set SDK overrides
     */

    /*
     * LR parameters:
     * 1) Epoch length,
     *      can this be directly taken from the ucode->m_inum
     * 2) Num of contexts used
     */
    rv = soc_sbx_t3p1_global_get(unit, "elen", epoch);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucodemgr_app_init: unit %d Reading Epoch len failed (%d), \n"), unit, rv));
        return rv;
    }

    rv = soc_sbx_t3p1_global_get(unit, "CONTEXTS", contexts);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucodemgr_app_init: unit %d Reading number of contexts used failed (%d) \n"),
                   unit, rv));
        return rv;
    }

    /* Set the microcode type */
    SOC_SBX_CONTROL(unit)->ucodetype = SOC_SBX_UCODE_TYPE_T3P1;

    sal_memcpy(sbx->uver_name,
               pkg->m_uver_name,
               strlen(pkg->m_uver_name));
    sbx->uver_maj   = pkg->m_uver_maj;
    sbx->uver_min   = pkg->m_uver_min;
    sbx->uver_patch = pkg->m_uver_patch;

    if (!soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype]) {
        soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype] =
            sal_alloc(64, "ucode ver");
        if (!soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype]) {
            return SOC_E_MEMORY;
        }
    }
    sal_sprintf(soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype], "%s %d.%d.%d",
                sbx->uver_name,
                sbx->uver_maj,
                sbx->uver_min,
                sbx->uver_patch);

    
    if ((sal_strlen(sbx->uver_name) == 5) &&
         (sal_strcmp(sbx->uver_name, "t3p1a")==0)) {
        soc_sbx_caladan3_sws_pr_icc_program_arad_header(unit, 0);
    } else {
        soc_sbx_caladan3_sws_pr_icc_program_sirius_header(unit, 0, 0, 0, 0);
    }

    return rv;
}

#endif

