/*
 * $Id: ramon_fe1600_property.c,v 1.8.64.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 PROPERTY
 */
#include <shared/bsl.h>
#include <soc/dnxf/fe1600/fe1600_property.h>
#include <soc/dnxf/cmn/dnxf_property.h>
#include <soc/dnxf/cmn/dnxf_drv.h>



#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_STAT

soc_dnxf_property_info_t soc_ramon_fe1600_property_info[]={

	{
		spn_MIIM_TIMEOUT_USEC,
		NULL,
		2000,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_MIIM_INTR_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_DEVICE_MODE,
		"SINGLE_STAGE_FE2",
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING,
	},
	{
		spn_IS_DUAL_MODE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SYSTEM_IS_VCS_128_IN_SYSTEM,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SYSTEM_IS_DUAL_MODE_IN_SYSTEM,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SYSTEM_IS_SINGLE_MODE_IN_SYSTEM,
		NULL,
		1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_SYSTEM_CONTAINS_MULTIPLE_PIPE_DEVICE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SYSTEM_IS_FE600_IN_SYSTEM,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_MERGE_CELLS,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},

	{
		spn_SERDES_MIXED_RATE_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_MULTICAST_MODE,
		"DIRECT",
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING
	},
	{
		spn_FABRIC_LOAD_BALANCING_MODE,
		"NORMAL_LOAD_BALANCE",
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING
	},
    {
		spn_FABRIC_CELL_FORMAT,
		"VSC256",
		FALSE,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING
	},
	{
		spn_FABRIC_TDM_FRAGMENT,
		NULL,
		0x181,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_TDM_OVER_PRIMARY_PIPE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_BIST_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_OPTIMIZE_PARTIAL_LINKS,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_VCS128_UNICAST_PRIORITY,
		NULL,
		-1 , /* get default value from soc_properties_init */
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_MAC_BUCKET_FILL_RATE,
		NULL,
		DNXF_FABRIC_MAC_BUCKET_FILL_RATE_DEFAULT,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SCHAN_TIMEOUT_USEC,
		NULL,
		-1, /* get default value from soc_properties_init */
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SCHAN_INTR_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_SYSTEM_REF_CORE_CLOCK,
		NULL,
		1200,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_CORE_CLOCK_SPEED,
		NULL,
		533,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_REPEATER_LINK_DEST,
		NULL,
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_BACKPLANE_SERDES_ENCODING,
		"KR_FEC",
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING,
	},
	{
		NULL,
		NULL,
		-1,
		-1,
	}

};

void
soc_ramon_fe1600_soc_properties_array_get(int unit,soc_dnxf_property_info_t** soc_dnxf_property_info_array )
{
	*soc_dnxf_property_info_array=soc_ramon_fe1600_property_info;
}
#undef _ERR_MSG_MODULE_NAME
