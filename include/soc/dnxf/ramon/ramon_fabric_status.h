/*
 * $Id: ramon_fabric_status.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON FABRIC STATUS H
 */
 
#ifndef _SOC_RAMON_FABRIC_STATUS_H_
#define _SOC_RAMON_FABRIC_STATUS_H_

#include <soc/dnxc/legacy/fabric.h>
#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/error.h>

soc_error_t soc_ramon_fabric_link_status_clear(int unit, soc_port_t link);
soc_error_t soc_ramon_fabric_link_status_get(int unit, soc_port_t link_id, uint32 *link_status, uint32 *errored_token_count);

#endif

