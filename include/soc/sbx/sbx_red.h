/*
 * $Id: sbx_red.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _BCM_SBXRED_H
#define _BCM_SBXRED_H

#include <bcm/error.h>

#include <bcm/types.h>
#include <bcm/port.h>


/* Switching Data Path Control
 * The SDK port API can be used to control individual data links within a 
 * system, including enabling and disabling links and determining status.  
 * However, a set of connected data links (SFI) will not be included in the
 * active switching behavior until they are explicitly enabled with a
 * soc_sbx_red_sfi_set() call is performed on the active BME unit
 *
 */
extern bcm_error_t soc_sbx_red_sfi_get( int unit, bcm_pbmp_t *sfi );
extern bcm_error_t soc_sbx_red_sfi_set(int unit, bcm_pbmp_t sfi );

extern bcm_error_t soc_sbx_red_bme_add( int unit, bcm_port_t port, int num,
                                           int active );
extern bcm_error_t soc_sbx_red_bme_delete( int unit, bcm_port_t port );



#endif
