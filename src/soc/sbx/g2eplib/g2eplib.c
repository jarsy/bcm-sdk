/*
 * $Id: g2eplib.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE2000 Ingress library Initialization
 */

#ifdef BCM_FE2000_SUPPORT
#include <shared/bsl.h>

#include <sal/types.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/glue.h>
#include <bcm_int/sbx/error.h>
#include "sbG2EplibContext.h"
#include "sbG2Eplib.h"

int
gu2_eplib_init(int unit)
{
    sbhandle pQeHdl;
    sbElibStatus_et nSts;
    sbG2EplibCtxt_pst pEgCtxt;
    sbG2EplibInitParams_pst sEplibPrm;
    uint32 erhType;
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    assert(sbx != NULL);

    if (!SOC_IS_SBX_QE(unit)) {
        LOG_CLI((BSL_META_U(unit,
                            "\nELib: Init allowed for QE only")));
        return SOC_E_INTERNAL;
    }

    if (!SOC_SBX_INIT(unit)) {
        LOG_CLI((BSL_META_U(unit,
                            "\nQE HW has not been initialized")));
        return SOC_E_INIT;
    }

    pQeHdl = sbx->sbhdl;
    pEgCtxt = (sbG2EplibCtxt_pst) sal_alloc(sizeof(*pEgCtxt), "EG context");
    if (NULL == pEgCtxt) {
        return SB_ELIB_MEM_ALLOC_FAIL;
    }

    sal_memset(pEgCtxt, 0, sizeof(*pEgCtxt));
    sbx->drv = pEgCtxt;

    sEplibPrm = (sbG2EplibInitParams_st *) sal_alloc(sizeof(sbG2EplibInitParams_st),
                                                     "EPlib params");
    if (NULL == sEplibPrm) {
        return SB_ELIB_MEM_ALLOC_FAIL;
    }

    sbG2EplibDefParamsGet(sEplibPrm);
    sEplibPrm->pHalCtx = pQeHdl;

    erhType = soc_property_get(unit, spn_QE_ERH_TYPE, SOC_SBX_G2P3_ERH_DEFAULT);
    switch (erhType) {
    case SOC_SBX_G2P3_ERH_QESS:
        pEgCtxt->eUcode = G2EPLIB_QESS_UCODE;
        break;
    case SOC_SBX_G2P3_ERH_DEFAULT:
        pEgCtxt->eUcode = G2EPLIB_UCODE;
        break;
    default:
        sal_free(sEplibPrm);
        return SOC_E_INIT;
    }

    nSts = (sbElibStatus_et) sbG2EplibInit(pEgCtxt, sEplibPrm /*pParams*/);
    if (nSts != SB_ELIB_OK) {
        LOG_CLI((BSL_META_U(unit,
                            "\nEpLib: Initialization failed\n")));
        sal_free(sEplibPrm);
        return nSts;
    }
    sbx->libInit = 1;  /* Set status to Initialized */
    sal_free(sEplibPrm);

    return SB_OK;
}
#else
int g2eplib_not_empty;
#endif
