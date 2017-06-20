/*
 * $Id: ramon_property.c,v 1.8.64.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON PROPERTY
 */
#include <shared/bsl.h>
#include <soc/dnxf/ramon/ramon_property.h>
#include <soc/dnxf/cmn/dnxf_property.h>
#include <soc/dnxf/cmn/dnxf_drv.h>



#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_STAT

soc_dnxf_property_info_t soc_ramon_property_info[]={ 

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
		spn_SYSTEM_IS_VCS_128_IN_SYSTEM,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_SYSTEM_CONTAINS_MULTIPLE_PIPE_DEVICE,
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
		spn_BIST_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_MAC_BUCKET_FILL_RATE,
		NULL,
		6, /* DNXF_FABRIC_MAC_BUCKET_FILL_RATE_DEFAULT */
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
		720,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_NUM_PIPES,
		NULL,
		1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FABRIC_PIPE_MAP,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_CUSTOM_FEATURE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_RATE_INT_MDIO_DIVIDEND,
		NULL,
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_RATE_INT_MDIO_DIVISOR,
		NULL,
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_SERDES_FABRIC_CLK_FREQ,
		NULL,
		1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_REPEATER_LINK_DEST,
		NULL,
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_REPEATER_LINK_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_FE_MC_PRIORITY_MAP_ENABLE,
		NULL,
		FALSE,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_FABRIC_TDM_PRIORITY_MIN,
		"3",
		FALSE,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING
	},

    /*
     * Fabric Cell FIFO DMA
     */
    {
		spn_FABRIC_CELL_FIFO_DMA_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_FABRIC_CELL_FIFO_DMA_TIMEOUT,
		NULL,
		1000,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_FABRIC_CELL_FIFO_DMA_THRESHOLD,
		NULL,
		2,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
    {
		spn_FABRIC_CELL_FIFO_DMA_BUFFER_SIZE,
		NULL,
		20480,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
	},
	{
		spn_FE_MC_ID_RANGE,
		"128K_HALF",
		-1,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING
	},
	{
		spn_FABRIC_LOCAL_ROUTING_ENABLE,
		NULL,
		0,
		SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT		
	},
    {
        spn_SYNC_ETH_CLK_TO_PORT_ID_CLK,
        NULL,
        0,
        SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
    },
    {
        spn_SYNC_ETH_CLK_DIVIDER,
        NULL,
        16,
        SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
    },
    {
        spn_BCM_RX_THREAD_PRI,
        NULL,
        0,
        SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT
    },
    {
		spn_BACKPLANE_SERDES_ENCODING,
		"RS_FEC",
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
soc_ramon_soc_properties_array_get(int unit,soc_dnxf_property_info_t** soc_dnxf_property_info_array )
{
	*soc_dnxf_property_info_array=soc_ramon_property_info;
}

#undef _ERR_MSG_MODULE_NAME

