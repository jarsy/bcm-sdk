/*
 * $Id: debug.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_DIAG
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/debug.h>
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/ARAD/arad_debug.h>

soc_error_t soc_dnx_dbg_egress_shaping_enable_get(const unsigned int unit, uint8 *enable)
{
  int rv = SOC_E_NONE;
    rv = jer2_arad_dbg_egress_shaping_enable_get_unsafe(unit, enable);
    DNXC_SAND_IF_ERR_RETURN(rv);
    return SOC_E_NONE;
}

soc_error_t soc_dnx_dbg_egress_shaping_enable_set(const unsigned int unit, const uint8 enable)
{
  int rv = SOC_E_NONE;
  rv = jer2_arad_dbg_egress_shaping_enable_set_unsafe(unit, enable);
  DNXC_SAND_IF_ERR_RETURN(rv);
  return SOC_E_NONE;
}

soc_error_t soc_dnx_dbg_flow_control_enable_get(const unsigned int unit, uint8 *enable)
{
  int rv = SOC_E_NONE;
  rv = jer2_arad_dbg_flow_control_enable_get_unsafe(unit, enable);
  DNXC_SAND_IF_ERR_RETURN(rv);
  return SOC_E_NONE;
}

soc_error_t soc_dnx_dbg_flow_control_enable_set(const unsigned int unit, const uint8 enable)
{
  int rv = SOC_E_NONE;
    rv = jer2_arad_dbg_flow_control_enable_set_unsafe(unit, enable);
    DNXC_SAND_IF_ERR_RETURN(rv);
    return SOC_E_NONE;
}

soc_error_t soc_dnx_compilation_vendor_valid(const unsigned int unit, const unsigned int val)
{

    return SOC_E_UNAVAIL;
}

#undef _ERR_MSG_MODULE_NAME

