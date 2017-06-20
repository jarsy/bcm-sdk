/*
 * $Id: device_wred.h,v 1.1.2.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        device_wred.h
 * Purpose:     Implement WRED configuration parameter algorithm. This could be
 *              shared across devices.
 */

#ifndef	_BCM_INT_SBX_DEVICE_WRED_H_
#define	_BCM_INT_SBX_DEVICE_WRED_H_

/* The following is the same for QE2000 and BM3200 device and has thus not been */
/* abstracted. This could change in the future.                                 */
#define BCM_SBX_DEVICE_WRED_MAX_RANGE_RANDOM_NUMBER    0x7FFFFF
#define BCM_SBX_DEVICE_WRED_MAX_SLOPE_VALUE            0xFFF

#define BCM_SBX_DEVICE_WRED_BM_WEIGHT_SHIFT            16
#define BCM_SBX_DEVICE_WRED_THRESHOLD_UNIT_SZ          16 /* size in bytes */


extern int
_bcm_sbx_device_wred_calc_config(int unit,
                                 int mtu_size,
                                 int max_queue_depth,
                                 bcm_cosq_gport_discard_t *discard,
                                 sbZfFabWredParameters_t *chip_params);

#endif	/* _BCM_INT_SBX_DEVICE_WRED_H_ */
