/*
 * $Id: device_wred.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        device_wred.c
 * Purpose:     Implement WRED configuration parameter algorithm. This could be
 *              shared across devices.
 */


#include <shared/bsl.h>

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/fabric/sbZfFabWredParameters.hx>
#include <soc/cm.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/device_wred.h>
#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>

int
_bcm_sbx_device_wred_calc_config(int unit,
                                 int mtu_size,
                                 int max_queue_depth,
                                 bcm_cosq_gport_discard_t *discard,
                                 sbZfFabWredParameters_t *chip_params)
{
    int                     rc = BCM_E_NONE;
    int                     i;
    uint8                 msb_queue_depth = 0, scaler;
    uint32                min_threshold, max_threshold, ecn_threshold;
    uint32                delta, pb2snd, pb_scale_slope, slope;


    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "WRED Parameters, MtuSz(0x%x) nMaxQueueDepth(0x%x) nMinThresholdInBytes(0x%x) nMaxThresholdInBytes(0x%x) DpAtMaxThrehold(0x%x) nEcnThreshold(0x%x)\n"),
                 mtu_size, max_queue_depth, discard->min_thresh, discard->max_thresh,
                 discard->drop_probability, discard->ecn_thresh));

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {   
	min_threshold = discard->min_thresh;
	max_threshold = discard->max_thresh;
	ecn_threshold = discard->ecn_thresh;
    } else {
	/* convert thresholds to units of 16 Bytes. Queue length messages are in units of 16 Bytes */
	max_queue_depth= max_queue_depth / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
	min_threshold = discard->min_thresh / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
	max_threshold = discard->max_thresh / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
	ecn_threshold = discard->ecn_thresh / BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ;
    }

    for (i = 0, msb_queue_depth = 0; i < 32; i++) {
        if ((max_queue_depth & (1 << i)) != 0) {
            msb_queue_depth = i;
        }
    }
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Max Bit Set in Queue Depth(0x%x)\n"),
                 msb_queue_depth));

    /* calculate scaler value */
    scaler = (msb_queue_depth > 15) ? (msb_queue_depth - 15) : 0;
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Scaler(0x%x)\n"),
                 scaler));

    /*
     * denominator calculation
     */
    delta = (max_threshold >> scaler) - (min_threshold >> scaler);
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "nDelta(0x%x)\n"),
                 delta));

    if (!delta) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s: Invalid config (delta = 0)\n"),
                   FUNCTION_NAME()));
        return BCM_E_CONFIG;
    }

    /*
     * numerator calculation
     */
    /* The factor "100" accounts for the drop probabilty representation as a percentage */
    pb2snd = (discard->drop_probability * BCM_SBX_DEVICE_WRED_MAX_RANGE_RANDOM_NUMBER) / (mtu_size * 100);
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "nPb2Snd(0x%x)\n"),
                 pb2snd));

    /* Accounts for BM weightage */
    pb_scale_slope = pb2snd << BCM_SBX_DEVICE_WRED_BM_WEIGHT_SHIFT;

    /*
     * slope calculation
     */
    slope = pb_scale_slope / delta;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "nPb2Snd(0x%x), nPbScaleSlope(0x%x), nSlope(0x%x)\n"),
                 pb2snd, pb_scale_slope , slope));

    if (slope > BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "WRED Slope(0x%x) Calculated to be greater then Maximum(0x%x)\n"),
                     slope, BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE));
        slope = BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE;
    }

    chip_params->m_nSlope = slope;
    chip_params->m_nScale = scaler;
    chip_params->m_nTmax = (max_threshold >> scaler);
    chip_params->m_nTmin = (min_threshold >> scaler);
    chip_params->m_nTecn = (ecn_threshold >> scaler);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "WRED Parameters, Slope(0x%x), Scale(0x%x), Tmax(0x%x) Tmin(0x%x) Tecn(0x%x)\n"),
                 chip_params->m_nSlope, chip_params->m_nScale,
                 chip_params->m_nTmax, chip_params->m_nTmin, chip_params->m_nTecn));

    return(rc);
}

