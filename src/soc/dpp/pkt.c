/*
 * $Id: pkt.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PKT
#include <shared/bsl.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/pkt.h>
#include <soc/dpp/drv.h>
#include <bcm_int/dpp/error.h>
#ifdef BCM_ARAD_SUPPORT
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#endif
#include <soc/dpp/PPD/ppd_api_trap_mgmt.h>
#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>

#ifdef BCM_ARAD_SUPPORT
#endif /* BCM_ARAD_SUPPORT */

#undef _ERR_MSG_MODULE_NAME

