/*
 * $Id: xbar.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _BCM_XBAR_H
#define _BCM_XBAR_H

#include <bcm/error.h>

#include <bcm/types.h>
#include <bcm/port.h>

typedef enum soc_sbx_xbar_mode_ {
    sbx_xbar_mode_normal,
    sbx_xbar_mode_configA,
    sbx_xbar_mode_configB,
    sbx_xbar_mode_hwSelect
} soc_sbx_xbar_mode_e;

/* Data Crossbar Routing Configuration API for SBX fabric technology 
 *
 * This API controls whether packets originating on a specific module are
 * allowed to send to the specified egress port
 */
extern bcm_error_t soc_sbx_xbar_config_set( int unit,
                                            bcm_port_t port,
                                            int modid,
                                            bcm_port_t xbport );

extern bcm_error_t soc_sbx_xbar_config_delete( int unit,
                                               bcm_port_t port,
                                               int modid );

extern bcm_error_t soc_sbx_xbar_config_get( int unit,
                                            bcm_port_t port,
                                            bcm_module_t modid,
                                            bcm_port_t *xbport );


bcm_error_t
soc_sbx_xbar_mode_set(int unit, soc_sbx_xbar_mode_e mode);

bcm_error_t
soc_sbx_xbar_mode_get(int unit, soc_sbx_xbar_mode_e *mode);


bcm_error_t
soc_sbx_xbar_fixed_config(int unit, int configAB, 
                          bcm_port_t xcfg[], int num_xcfgs);


#endif
