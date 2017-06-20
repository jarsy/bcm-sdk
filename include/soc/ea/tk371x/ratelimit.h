/*
 * $Id: ratelimit.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     ratelimit.h
 * Purpose:
 *
 */
#ifndef _SOC_EA_RATELIMIT_H
#define _SOC_EA_RATELIMIT_H

#include <soc/ea/tk371x/CtcRateLimitApi.h>

typedef EthPortUsPolicingPa _soc_ea_eth_port_us_policing_pa_t;
typedef EthPortDSRateLimitingPa _soc_ea_eth_port_ds_rate_limiting_pa_t;
typedef CtcEthPortUSPolicingPa _soc_ea_ctc_eth_port_us_policing_pa_t;
typedef CtcEthPortDSRateLimitingPa _soc_ea_ctc_eth_port_ds_rate_limiting_pa_t;

#define _soc_ea_ctc_eth_port_us_policings_set 	CtcEthPortSetUSPolicings
#define _soc_ea_ctc_eth_port_us_policings_get	CtcEthPortGetUSPolicings
#define _soc_ea_ctc_eth_port_ds_rate_limiting_set	CtcEthPortSetDSRateLimiting
#define _soc_ea_ctc_eth_port_ds_rate_limiting_get	CtcEthPortGetDSRateLimiting

#endif /* _SOC_EA_RATELIMIT_H */
