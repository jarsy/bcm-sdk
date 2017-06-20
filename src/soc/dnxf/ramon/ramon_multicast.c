/*
 * $Id: ramon_multicast.c,v 1.6.34.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON MULTICAST
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MCAST

#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/ramon/ramon_multicast.h>

#include <soc/dnxf/cmn/dnxf_drv.h>

soc_error_t
soc_ramon_multicast_mode_get(int unit, soc_dnxf_multicast_table_mode_t* multicast_mode)
{
	DNXC_INIT_FUNC_DEFS;
	*multicast_mode = soc_dnxf_multicast_table_mode_128k_half; /* default */
	switch(SOC_DNXF_CONFIG(unit).fe_mc_id_range)
	{
		case soc_dnxf_multicast_table_mode_64k:
		case soc_dnxf_multicast_table_mode_128k:
		case soc_dnxf_multicast_table_mode_128k_half:
			*multicast_mode =  SOC_DNXF_CONFIG(unit).fe_mc_id_range;
			break;
		default:
			DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Wrong mc_table_mode value %d"), SOC_DNXF_CONFIG(unit).fe_mc_id_range));
	}
exit:
	DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_multicast_table_size_get(int unit, uint32* mc_table_size)
{
	soc_dnxf_multicast_table_mode_t multicast_mode;
	DNXC_INIT_FUNC_DEFS;
	DNXC_IF_ERR_EXIT(soc_ramon_multicast_mode_get(unit, &multicast_mode));
	switch (multicast_mode)
	{
		case soc_dnxf_multicast_table_mode_64k:
			*mc_table_size = 64*1024;
			break;
		case soc_dnxf_multicast_table_mode_128k:
			*mc_table_size = 128*1024;
			break;
		case soc_dnxf_multicast_table_mode_128k_half:
			*mc_table_size = 128*1024;
			break;
		default:
			DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("wrong mc_table_mode value %d"),SOC_DNXF_CONFIG(unit).fe_mc_id_range));
            break;
	}
exit:
	DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_multicast_table_entry_size_get(int unit, uint32* entry_size)
{
	soc_dnxf_multicast_table_mode_t multicast_mode;
	DNXC_INIT_FUNC_DEFS;
	DNXC_IF_ERR_EXIT(soc_ramon_multicast_mode_get(unit, &multicast_mode));
	switch (multicast_mode)
	{
		case soc_dnxf_multicast_table_mode_64k:
			*entry_size = 144;
			break;
		case soc_dnxf_multicast_table_mode_128k:
			*entry_size = 144;
			break;
		case soc_dnxf_multicast_table_mode_128k_half:
			*entry_size = 72;
			break;
		default:
			DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("wrong mc_table_mode value %d"),SOC_DNXF_CONFIG(unit).fe_mc_id_range));
	}
exit:
	DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME
