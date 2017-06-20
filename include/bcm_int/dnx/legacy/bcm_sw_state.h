/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * INFO: this module is the entry point the branch of the sw state that compose the dnx bcm 
 * layer's sw state 
 *  
 */
#ifndef _DNX_BCM_SW_STATE_H
#define _DNX_BCM_SW_STATE_H

#if defined(BCM_DNX_SUPPORT)
/* { */
#include <soc/types.h>
#include <soc/error.h>
#include <shared/swstate/sw_state.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>
#include <bcm_int/dnx/legacy/alloc_mngr.h>
#include <bcm_int/dnx/legacy/alloc_mngr_utils.h>
#include <bcm_int/dnx/legacy/switch.h>

typedef struct soc_dnx_bcm_sw_state_s{
    PARSER_HINT_PTR bcm_dnx_gport_mgmt_info_t            *gport_mgmt;
    PARSER_HINT_PTR bcm_dnx_alloc_mngr_info_t            *alloc_mngr;
    PARSER_HINT_PTR bcm_dnx_alloc_mngr_utils_info_t      *alloc_mngr_utils;
    PARSER_HINT_PTR bcm_dnx_switch_info_t                *_switch;
    PARSER_HINT_PTR bcm_dnx_cosq_info_t                  *cosq;
} soc_dnx_bcm_sw_state_t;
/* } */
#endif
#endif /* _DNX_SHR_SW_STATE_H */
