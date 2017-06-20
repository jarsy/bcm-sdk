/*
* $Id$
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
*/

#ifndef _SOC_DPP_DEBUG_H
#define _SOC_DPP_DEBUG_H

#include <soc/dpp/ARAD/arad_debug.h>
#include <soc/error.h>

soc_error_t soc_dpp_dbg_egress_shaping_enable_get(const unsigned int unit, uint8 *enable);
soc_error_t soc_dpp_dbg_egress_shaping_enable_set(const unsigned int unit, const uint8 enable);
soc_error_t soc_dpp_dbg_flow_control_enable_get(const unsigned int unit, uint8 *enable);
soc_error_t soc_dpp_dbg_flow_control_enable_set(const unsigned int unit, const uint8 enable);
soc_error_t soc_dpp_compilation_vendor_valid(const unsigned int unit, const unsigned int val);

#ifndef _KERNEL_ 
#endif /*_KERNEL_*/
#endif /*_SOC_DPP_DEBUG_H*/
