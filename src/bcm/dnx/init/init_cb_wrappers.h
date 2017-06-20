/** \file init_cb_wrappers.h
 * 
 * The DNX init sequence uses CB functions for init and deinit steps. new functions are written according the required 
 * definitions, old ones however are wrapped and placed in this file.
 * 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_DNX_INIT_INITCBWRAPPERS_H_INCLUDED
/*
 * { 
 */
#define _BCM_DNX_INIT_INITCBWRAPPERS_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm_int/dnx/init/init.h>

/**
 * \brief - init Done step, used to update relevant modules that 
 *        the init was done ( should be used only for this
 *        purpose ) if this function starts to explode out of
 *        control need to split into actual different steps.
 *
 */
shr_error_e dnx_init_done_init(
    int unit);

/**
 * \brief - currently place holder, probably won't be 
 *        implemented at all.
 *
 */
shr_error_e dnx_init_done_deinit(
    int unit);

/**
 * \brief - wrapper for soc feature init function
 *
 */
shr_error_e dnx_init_feature_init(
    int unit);

/**
 * \brief - place holder for feature deinit function
 *
 */
shr_error_e dnx_init_feature_deinit(
    int unit);

#ifdef CMODEL_SERVER_MODE
/**
 * \brief - wrapper for cmodel_reg_access_init
 *
 */
shr_error_e dnx_init_cmodel_reg_access_init(
    int unit);

/**
 * \brief - wrapper for cmodel_reg_access_deinit
 *
 */
shr_error_e dnx_init_cmodel_reg_access_deinit(
    int unit);
#endif

/**
 * \brief - init RX module
 *
 */
shr_error_e dnx_init_rx_init(
    int unit);

/**
 * \brief - place holder for RX module deinit func once 
 * implemented 
 *
 */
shr_error_e dnx_init_rx_deinit(
    int unit);

/**
 * \brief - wrapper for dnx_init_l3_algo_init
 *
 */
shr_error_e dnx_init_l3_algo_init(
    int unit);

/**
 * \brief - place holder for l3 algo res module deinit func
 * once implemented
 *
 */
shr_error_e dnx_init_l3_algo_deinit(
    int unit);

/**
 * \brief - wrapper for dnx_init_l3_module_init
 *
 */
shr_error_e dnx_init_l3_module_init(
    int unit);

/**
 * \brief - place holder for l3 module deinit func
 * once implemented
 *
 */
shr_error_e dnx_init_l3_module_deinit(
    int unit);

/**
 * \brief - wrapper for dnx_init_pp_port_module_init
 */
shr_error_e dnx_init_pp_port_init(
    int unit);

/**
 * \brief - place holder for pp_port module deinit func
 * once implemented
 */
shr_error_e dnx_init_pp_port_deinit(
    int unit);

/**
 * \brief - HW overwrites
 *
 */
shr_error_e dnx_init_hw_overwrite_init(
    int unit);

/**
 * \brief - wrapper for PEMLA drv initialization
 *
 */
shr_error_e dnx_init_pemla_init(
    int unit);

/*
 * } 
 */
#endif /* _BCM_DNX_INIT_INITCBWRAPPERS_H_INCLUDED */
