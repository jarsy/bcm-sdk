/*
 * $Id: dfe_port.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DFE PORT H
 */
 
#ifndef _SOC_DFE_PORT_H_
#define _SOC_DFE_PORT_H_

#include <soc/types.h>
#include <soc/error.h>
#include <soc/dcmn/dcmn_defs.h>

typedef enum soc_dfe_port_update_type_s
{
    soc_dfe_port_update_type_sfi,
    soc_dfe_port_update_type_port,
    soc_dfe_port_update_type_all,
    soc_dfe_port_update_type_sfi_disabled,
    soc_dfe_port_update_type_port_disabled,
    soc_dfe_port_update_type_all_disabled
} soc_dfe_port_update_type_t;


/**********************************************************/
/*                     Functions                          */
/**********************************************************/

soc_error_t soc_dfe_port_loopback_set(int unit, soc_port_t port, soc_dcmn_loopback_mode_t loopback);
soc_error_t soc_dfe_port_loopback_get(int unit, soc_port_t port, soc_dcmn_loopback_mode_t* loopback);


#endif /*_SOC_DFE_PORT_H_*/
