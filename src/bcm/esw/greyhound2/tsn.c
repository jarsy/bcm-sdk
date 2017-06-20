/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <soc/drv.h>
#include <bcm/tsn.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <shared/bsl.h>
#include <bcm_int/esw/greyhound2.h>

#if defined(BCM_GREYHOUND2_SUPPORT) && defined(BCM_TSN_SUPPORT)

/* Initialization flag for GH2 TSN bookkeeping info structure */
static int gh2_tsn_bkinfo_initialized = 0;

/* Structure definition for GH2 TSN bookkeeping info */
typedef struct gh2_tsn_bkinfo_s {
    int tmp; /* unused variable, remove after implementation is in */
} gh2_tsn_bkinfo_t;

/* GH2 TSN bookkeeping info */
STATIC gh2_tsn_bkinfo_t *gh2_tsn_bkinfo[BCM_MAX_NUM_UNITS] = {NULL};

/* Check if TSN bookkeeping info structure intialized */
#define TSN_BKINFO_IS_INIT(_unit_)                                        \
    do {                                                                  \
        if (gh2_tsn_bkinfo[(_unit_)] == NULL) {                           \
            LOG_ERROR(BSL_LS_BCM_TSN,                                     \
                      (BSL_META_U((_unit_),                               \
                                  "TSN Error: chip bookkeeping info not " \
                                  "initialized\n")));                     \
            return (BCM_E_INIT);                                          \
        }                                                                 \
    } while (0)

/* For TSN bookkeeping access */
#define TSN_BKINFO(_u_) gh2_tsn_bkinfo[_u_]

/*
 * Function:
 *     bcmi_gh2_tsn_control_set
 * Description:
 *     Configure device-wide operation modes for TSN.
 * Parameters:
 *     unit - (IN) Unit number.
 *     type - (IN) Control type of bcm_tsn_control_t.
 *     arg  - (IN) argument to be set
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
bcmi_gh2_tsn_control_set(int unit, bcm_tsn_control_t type, uint32 arg)
{
#if defined(BCM_TSN_SR_SUPPORT)
    soc_reg_t config_reg = INVALIDr;
    soc_mem_t config_mem = INVALIDm;
    soc_field_t config_field = INVALIDf;
    uint32 fldval = 0;
    uint32 regval = 0;
    int memidx = 0;
    uint32 entry[SOC_MAX_MEM_WORDS];
#endif /* BCM_TSN_SR_SUPPORT */

    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_control_set: type=%d arg=%d\n"),
                            type, arg));

#if defined(BCM_TSN_SR_SUPPORT)
    if (!soc_feature(unit, soc_feature_tsn_sr)) {
        return (BCM_E_UNAVAIL);
    }
#endif /* BCM_TSN_SR_SUPPORT */

    switch (type) {
#if defined(BCM_TSN_SR_SUPPORT)
        case bcmTsnControlSrAgeTickInterval:
            if (((int)arg < 0) || (arg > 0xffff)) {
                return BCM_E_PARAM;
            }
            config_reg = SR_AGING_TIMERr;
            config_field = AGE_OUT_TIMEf;
            fldval = arg;
            break;
        case bcmTsnControlSrAgeOutEnable:
            if (((int)arg < 0) || (arg > 1)) {
                return BCM_E_PARAM;
            }
            config_reg = SR_AGING_TIMERr;
            config_field = AGE_ENAf;
            fldval = arg;
            break;
        case bcmTsnControlSrSeqNumWindowResetMode:
            config_reg = SR_AGING_TIMERr;
            config_field = SN_WINDOW_RESET_TYPEf;
            if (arg == BCM_TSN_CONTROL_SR_SEQNUM_WINDOW_RESET_MODE_HW1) {
                fldval = 0;
            } else if (arg == BCM_TSN_CONTROL_SR_SEQNUM_WINDOW_RESET_MODE_HW2) {
                fldval = 1;
            } else if (arg == BCM_TSN_CONTROL_SR_SEQNUM_WINDOW_RESET_MODE_SW) {
                fldval = 2;
            } else {
                return BCM_E_PARAM;
            }
            break;
        case bcmTsnControlSrLooseOutOfOrder:
            if (((int)arg < 0) || (arg > 1)) {
                return BCM_E_PARAM;
            }
            config_reg = SR_OUT_OF_ORDER_CONTROLr;
            config_field = OUT_OF_ORDER_CONTROLf;
            fldval = arg;
            break;
        case bcmTsnControlSrHsrEthertype:
            if (arg > 0xffff) {
                return BCM_E_PARAM;
            }
            config_mem = SR_ETHERTYPESm;
            config_field = HSR_ETHERTYPEf;
            memidx = 0;
            fldval = arg;
            break;
        case bcmTsnControlSrPrpEthertype:
            if (arg > 0xffff) {
                return BCM_E_PARAM;
            }
            config_mem = SR_ETHERTYPESm;
            config_field = PRP_SUFFIXf;
            memidx = 0;
            fldval = arg;
            break;
        case bcmTsnControlSrDot1cbEthertype:
            if (arg > 0xffff) {
                return BCM_E_PARAM;
            }
            config_mem = SR_ETHERTYPESm;
            config_field = DOT1CB_ETHERTYPEf;
            memidx = 0;
            fldval = arg;
            break;
#endif /* BCM_TSN_SR_SUPPORT */
        default:
            return BCM_E_PARAM;
    }

#if defined(BCM_TSN_SR_SUPPORT)
    if (config_reg != INVALIDr) {
        BCM_IF_ERROR_RETURN
            (soc_reg32_get(unit, config_reg, REG_PORT_ANY, 0, &regval));
        soc_reg_field_set(unit, config_reg, &regval,
                          config_field, fldval);
        BCM_IF_ERROR_RETURN
            (soc_reg32_set(unit, config_reg, REG_PORT_ANY, 0, regval));
    }

    if (config_mem != INVALIDm) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, config_mem, MEM_BLOCK_ANY, memidx, &entry));
        soc_mem_field32_set(unit, config_mem, &entry, config_field, fldval);
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, config_mem, MEM_BLOCK_ALL, memidx, &entry));
    }
#endif /* BCM_TSN_SR_SUPPORT */
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcmi_gh2_tsn_control_get
 * Description:
 *     Get the configure of device-wide TSN operation modes.
 * Parameters:
 *     unit - (IN) Unit number.
 *     type - (IN) Control type of bcm_tsn_control_t.
 *     arg  - (OUT) argument got
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
bcmi_gh2_tsn_control_get(int unit, bcm_tsn_control_t type, uint32 *arg)
{
#if defined(BCM_TSN_SR_SUPPORT)
    uint32 fldval = 0;
    uint32 regval = 0;
    uint32 entry[SOC_MAX_MEM_WORDS];
#endif /* BCM_TSN_SR_SUPPORT */

    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_control_get: type=%d "),
                            type));

#if defined(BCM_TSN_SR_SUPPORT)
    if (!soc_feature(unit, soc_feature_tsn_sr)) {
        return (BCM_E_UNAVAIL);
    }
#endif /* BCM_TSN_SR_SUPPORT */

    /* Paramter validation */
    if (NULL == arg) {
        return BCM_E_PARAM;
    }

    switch (type) {
#if defined(BCM_TSN_SR_SUPPORT)
        case bcmTsnControlSrAgeTickInterval:
            BCM_IF_ERROR_RETURN(READ_SR_AGING_TIMERr(unit, &regval));
            fldval = soc_reg_field_get(unit, SR_AGING_TIMERr,
                                       regval, AGE_OUT_TIMEf);
            break;
        case bcmTsnControlSrAgeOutEnable:
            BCM_IF_ERROR_RETURN(READ_SR_AGING_TIMERr(unit, &regval));
            fldval = soc_reg_field_get(unit, SR_AGING_TIMERr,
                                       regval, AGE_ENAf);
            break;
        case bcmTsnControlSrSeqNumWindowResetMode:
            BCM_IF_ERROR_RETURN(READ_SR_AGING_TIMERr(unit, &regval));
            fldval = soc_reg_field_get(unit, SR_AGING_TIMERr,
                                       regval, SN_WINDOW_RESET_TYPEf);
            if (fldval == 0) {
                fldval = BCM_TSN_CONTROL_SR_SEQNUM_WINDOW_RESET_MODE_HW1;
            } else if (fldval == 1) {
                fldval = BCM_TSN_CONTROL_SR_SEQNUM_WINDOW_RESET_MODE_HW2;
            } else if (fldval == 2) {
                fldval = BCM_TSN_CONTROL_SR_SEQNUM_WINDOW_RESET_MODE_SW;
            } else {
                return BCM_E_FAIL;
            }
            break;
        case bcmTsnControlSrLooseOutOfOrder:
            BCM_IF_ERROR_RETURN(READ_SR_OUT_OF_ORDER_CONTROLr(unit, &regval));
            fldval = soc_reg_field_get(unit, SR_OUT_OF_ORDER_CONTROLr,
                                       regval, OUT_OF_ORDER_CONTROLf);
            break;
        case bcmTsnControlSrHsrEthertype:
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, SR_ETHERTYPESm, MEM_BLOCK_ANY, 0, &entry));
            soc_mem_field_get(unit, SR_ETHERTYPESm,
                              entry, HSR_ETHERTYPEf, &fldval);
            break;
        case bcmTsnControlSrPrpEthertype:
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, SR_ETHERTYPESm, MEM_BLOCK_ANY, 0, &entry));
            soc_mem_field_get(unit, SR_ETHERTYPESm,
                              entry, PRP_SUFFIXf, &fldval);
            break;
        case bcmTsnControlSrDot1cbEthertype:
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, SR_ETHERTYPESm, MEM_BLOCK_ANY, 0, &entry));
            soc_mem_field_get(unit, SR_ETHERTYPESm,
                              entry, DOT1CB_ETHERTYPEf, &fldval);
            break;
#endif /* BCM_TSN_SR_SUPPORT */
        default:
            return BCM_E_PARAM;
    }

#if defined(BCM_TSN_SR_SUPPORT)
    *arg = fldval;

    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "arg=%d\n"),
                            *arg));
#endif /* BCM_TSN_SR_SUPPORT */
    return BCM_E_NONE;
}

/* Per-chip control driver set */
STATIC const tsn_chip_ctrl_info_t gh2_chip_ctrl_info = {
    /* per-chip control drivers */
    bcmi_gh2_tsn_control_set,
    bcmi_gh2_tsn_control_get
};

/* GH2 TSN device info */
STATIC const bcmi_esw_tsn_dev_info_t gh2_tsn_devinfo = {
    &gh2_chip_ctrl_info
};

/*
 * Function:
 *     bcmi_gh2_bkinfo_cleanup
 * Purpose:
 *     Clear bookkeeping.
 * Parameters:
 *     unit             - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 *
 */
STATIC int
bcmi_gh2_bkinfo_cleanup(int unit)
{
    /* Free and clear the bookkeeping info structure */
    if (gh2_tsn_bkinfo[unit] != NULL) {
        TSN_FREE(unit, gh2_tsn_bkinfo[unit], 0);
        gh2_tsn_bkinfo[unit] = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcmi_gh2_tsn_info_init
 * Description:
 *     Setup the chip specific info.
 * Parameters:
 *     unit    - (IN) Unit number.
 *     devinfo - (OUT) device info structure.
 * Returns:
 *     BCM_E_XXX
 */
int
bcmi_gh2_tsn_info_init(
    int unit,
    const bcmi_esw_tsn_dev_info_t **devinfo)
{
    gh2_tsn_bkinfo_t *bkinfo = NULL;
    int rv = BCM_E_NONE;
    int i;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (devinfo == NULL) {
        return BCM_E_PARAM;
    }

    /* GH2 TSN bookkeeping info initialization */
    if (!gh2_tsn_bkinfo_initialized) {
        for (i = 0; i < BCM_MAX_NUM_UNITS; i++) {
            gh2_tsn_bkinfo[i] = NULL;
        }
        gh2_tsn_bkinfo_initialized = 1;
    }

    /* Clean up the resource */
    if (gh2_tsn_bkinfo[unit] != NULL) {
        rv = bcmi_gh2_bkinfo_cleanup(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    /* Allocate bookkeeping info resource */
    /* coverity[bad_memset] */
    TSN_ALLOC(unit, bkinfo, gh2_tsn_bkinfo_t,
              sizeof(gh2_tsn_bkinfo_t),
              "GH2 TSN bookkeeping info", 0, rv);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    gh2_tsn_bkinfo[unit] = bkinfo;

    /* Return the chip info structure */
    *devinfo = &gh2_tsn_devinfo;

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcmi_gh2_tsn_info_detach
 * Description:
 *     Detach the chip specific info.
 * Parameters:
 *     unit    - (IN) Unit number.
 * Returns:
 *     BCM_E_XXX
 */
int
bcmi_gh2_tsn_info_detach(int unit)
{
    /* Detach bookkeeping info */
    BCM_IF_ERROR_RETURN(bcmi_gh2_bkinfo_cleanup(unit));

    return BCM_E_NONE;
}

#endif /* BCM_GREYHOUND2_SUPPORT && BCM_TSN_SUPPORT */
