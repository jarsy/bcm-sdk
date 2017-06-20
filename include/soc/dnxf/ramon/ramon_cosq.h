/*
 * $Id: ramon_fabric_cell.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON COSQ H
 */


#ifndef _SOC_RAMON_COSQ_H_
#define _SOC_RAMON_COSQ_H_

/**********************************************************/
/*                  Includes                              */
/**********************************************************/

#include <soc/error.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/types.h>

/**********************************************************/
/*                  Defines                               */
/**********************************************************/
#define SOC_RAMON_COSQ_WFQ_WEIGHT_MAX                127
#define SOC_RAMON_COSQ_WFQ_WEIGHT_IGNORE             1
#define SOC_RAMON_COSQ_WFQ_WEIGHT_NOF_BITS           7

#define SOC_RAMON_COSQ_WFQ_DCH_THRESH_MAX            511
#define SOC_RAMON_COSQ_WFQ_DCM_THRESH_MAX            2047
#define SOC_RAMON_COSQ_WFQ_DCL_THRESH_MAX            1023

#define SOC_RAMON_COSQ_RATE_DCH_PERIOD_MAX           0xf
#define SOC_RAMON_COSQ_RATE_DCL_DEC_INC_VAL_MAX      0xff
#define SOC_RAMON_COSQ_RATE_DCL_DEC_INC_NOF_BITS     8
/**********************************************************/
/*                  Functions                             */
/**********************************************************/

/* pipe wfq weight functions */
soc_error_t  soc_ramon_cosq_pipe_rx_weight_set(int unit, int pipe, int port, soc_dnxf_cosq_weight_mode_t mode, int weight);
soc_error_t  soc_ramon_cosq_pipe_rx_weight_get(int unit, int pipe, int port, soc_dnxf_cosq_weight_mode_t mode, int *weight);
soc_error_t  soc_ramon_cosq_pipe_mid_weight_set(int unit, int pipe, int port, soc_dnxf_cosq_weight_mode_t mode, int weight);
soc_error_t  soc_ramon_cosq_pipe_mid_weight_get(int unit, int pipe, int port, soc_dnxf_cosq_weight_mode_t mode, int *weight);
soc_error_t  soc_ramon_cosq_pipe_tx_weight_set(int unit, int pipe, int port, soc_dnxf_cosq_weight_mode_t mode, int weight);
soc_error_t  soc_ramon_cosq_pipe_tx_weight_get(int unit, int pipe, int port, soc_dnxf_cosq_weight_mode_t mode, int *weight);

/* pipe wfq threshold functions*/
soc_error_t  soc_ramon_cosq_pipe_rx_threshold_set(int unit, int pipe, int port, int threshold);
soc_error_t  soc_ramon_cosq_pipe_rx_threshold_get(int unit, int pipe, int port, int *threshold);
soc_error_t  soc_ramon_cosq_pipe_mid_threshold_set(int unit, int pipe, int port, int threshold);
soc_error_t  soc_ramon_cosq_pipe_mid_threshold_get(int unit, int pipe, int port, int *threshold);
soc_error_t  soc_ramon_cosq_pipe_tx_threshold_set(int unit, int pipe, int port, int threshold);
soc_error_t  soc_ramon_cosq_pipe_tx_threshold_get(int unit, int pipe, int port, int *threshold);

/* pipe rate fucntions */
soc_error_t  soc_ramon_cosq_pipe_rx_rate_set(int unit, int pipe, int port, soc_dnxf_cosq_shaper_mode_t shaper_mode, uint32 rate);
soc_error_t  soc_ramon_cosq_pipe_rx_rate_get(int unit, int pipe, int port, soc_dnxf_cosq_shaper_mode_t *shaper_mode, uint32 *rate);
soc_error_t  soc_ramon_cosq_pipe_tx_rate_set(int unit, int port, soc_dnxf_cosq_shaper_mode_t shaper_mode, uint32 rate);
soc_error_t  soc_ramon_cosq_pipe_tx_rate_get(int unit, int port, soc_dnxf_cosq_shaper_mode_t *shaper_mode, uint32 *rate);
soc_error_t  soc_ramon_cosq_pipe_tx_rate_enable_set(int unit, int pipe, int port, int enable);
soc_error_t  soc_ramon_cosq_pipe_tx_rate_enable_get(int unit, int pipe, int port, int *enable);

#endif /*!_SOC_RAMON_COSQ_H_*/

