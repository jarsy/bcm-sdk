/* $Id: jer_pp_metering.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER_PP_METERING_INCLUDED__
/* { */
#define __JER_PP_METERING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_framework.h> 
#include <soc/error.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_metering.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

typedef struct
{
  int in_pp_port_size_delta;
} __ATTRIBUTE_PACKED__ JER_PACKET_SIZE_PROFILES_TBL_DATA;

#define JER_PP_SW_DB_MULTI_SET_PACKET_SIZE_PROFILE_NOF_MEMBER 8


/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
* NAME:
*     jer_pp_metering_init
* FUNCTION:
*     Initialization of the Jericho metering.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer_pp_metering_init(
    SOC_SAND_IN  int  unit
  );

/*********************************************************************
* NAME:
*     jer_pp_metering_init_mrps_config
* FUNCTION:
*     Global configuration of the Jericho MRPS metering.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/

uint32
  jer_pp_metering_init_mrps_config(
		SOC_SAND_IN int unit
  );
/*********************************************************************
* NAME:
*     jer_pp_metering_init_mrpsEm_config
* FUNCTION:
*     Global configuration of the Jericho MRPS_EM Eth. Policer.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer_pp_metering_init_mrpsEm_config(
		SOC_SAND_IN int unit
  );

/*********************************************************************
* NAME:
*     jer_pp_Eth_policer_pcd_init
* FUNCTION:
*     Global configuration of the Jericho MRPS_EM PCD MAP table Eth. Policer.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32 
  jer_pp_eth_policer_pcd_init(
     SOC_SAND_IN int unit
  );

/*********************************************************************
* NAME:
*     soc_jer_pp_mtr_policer_global_sharing_get
* FUNCTION:
*     gets the global_sharing rate flag with respect to GLOBAL_SHARINGf
* INPUT:
*   int                         unit				- Identifier of the device to access
*   int                         core_id				- specify meter core - for MRPS block
*	int        					meter_id			- specify meter id
*	int							meter_group			- specify meter memory section
*	uint32* 					global_sharing_ptr
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the reading info sequence.
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
soc_error_t
  soc_jer_pp_mtr_policer_global_sharing_get(
    int                         unit,
	int                         core_id,
	int        					meter_id,
	int							meter_group,
	uint32* 					global_sharing_ptr
);

/*********************************************************************
* NAME:
*     soc_jer_pp_mtr_policer_global_sharing_set
* FUNCTION:
*     sets the global_sharing rate flag with respect to GLOBAL_SHARINGf
* INPUT:
*   int                         unit				- Identifier of the device to access
*   int                         core_id				- specify meter core - for MRPS block
*	int        					meter_id			- specify meter id
*	int							meter_group			- specify meter memory section
*	uint32* 					global_sharing_ptr
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the reading info sequence.
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
soc_error_t
  soc_jer_pp_mtr_policer_global_sharing_set(
    int                         unit,
	int                         core_id,
	int        					meter_id,
	int							meter_group,
	uint32* 					global_sharing_ptr
);

/*********************************************************************
* NAME:
*     jer_pp_mtr_eth_policer_params_set
* FUNCTION:
*     sets the local Eth. policer on a specific port
* INPUT:
*   int                         	unit				- Identifier of the device to access
*   SOC_PPC_PORT                    port_ndx			- specify port index to meter
*	SOC_PPC_MTR_ETH_TYPE        	eth_type_ndx		- specify packet's type to meter
*	SOC_PPC_MTR_BW_PROFILE_INFO		policer_info		- specify Eth. policer parameters
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the reading info sequence.
*   Called from bcm_petra_rate_bandwidth_set().
*********************************************************************/
uint32
  jer_pp_mtr_eth_policer_params_set(
    SOC_SAND_IN  int                                    unit,
	SOC_SAND_IN  int                           			core_id,
    SOC_SAND_IN  SOC_PPC_PORT                           port_ndx,
	SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                   eth_type_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO            *policer_info
  );

/*********************************************************************
* NAME:
*     jer_pp_mtr_eth_policer_params_get
* FUNCTION:
*     gets the local Eth. policer on a specific port
* INPUT:
*   int                         	unit				- Identifier of the device to access
*   SOC_PPC_PORT                    port_ndx			- specify port index to meter
*	SOC_PPC_MTR_ETH_TYPE        	eth_type_ndx		- specify packet's type to meter
* OUTPUT:
*	SOC_PPC_MTR_BW_PROFILE_INFO		*policer_info		- specify Eth. policer parameters
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer_pp_mtr_eth_policer_params_get(
    SOC_SAND_IN  int                        	unit,
	SOC_SAND_IN  int                           core_id,
    SOC_SAND_IN  SOC_PPC_PORT                   port_ndx,
	SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE           eth_type_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO    *policer_info
  );

/*********************************************************************
* NAME:
*     jer_pp_mtr_eth_policer_params_get
* FUNCTION:
*     gets the local Eth. policer on a specific port
* INPUT:
*   int                         	unit				- Identifier of the device to access
*   SOC_PPC_PORT                    port_ndx			- specify port index to meter
*	SOC_PPC_MTR_ETH_TYPE        	eth_type_ndx		- specify packet's type to meter
* OUTPUT:
*	SOC_PPC_MTR_BW_PROFILE_INFO		*policer_info		- specify Eth. policer parameters
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer_pp_mtr_eth_policer_glbl_profile_set(
	SOC_SAND_IN int       						unit,
	SOC_SAND_IN  int                           core_id,
	SOC_SAND_IN uint32	                		policer_ndx,
	SOC_SAND_IN SOC_PPC_MTR_BW_PROFILE_INFO    	*policer_info
  );

/*********************************************************************
* NAME:
*     jer_pp_mtr_eth_policer_params_get
* FUNCTION:
*     gets the local Eth. policer on a specific port
* INPUT:
*   int                         	unit				- Identifier of the device to access
*   SOC_PPC_PORT                    port_ndx			- specify port index to meter
*	SOC_PPC_MTR_ETH_TYPE        	eth_type_ndx		- specify packet's type to meter
* OUTPUT:
*	SOC_PPC_MTR_BW_PROFILE_INFO		*policer_info		- specify Eth. policer parameters
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32  
  jer_pp_mtr_eth_policer_glbl_profile_get(
	 SOC_SAND_IN  int                      		unit,
	 SOC_SAND_IN  int                           core_id,
	 SOC_SAND_IN  uint32                  		glbl_profile_idx,
	 SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO 	*policer_info
  );

/*********************************************************************
* NAME:
*     jer_pp_mtr_eth_policer_params_get
* FUNCTION:
*     gets the local Eth. policer on a specific port
* INPUT:
*   int                         	unit				- Identifier of the device to access
*   SOC_PPC_PORT                    port_ndx			- specify port index to meter
*	SOC_PPC_MTR_ETH_TYPE        	eth_type_ndx		- specify packet's type to meter
* OUTPUT:
*	SOC_PPC_MTR_BW_PROFILE_INFO		*policer_info		- specify Eth. policer parameters
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32  
  jer_pp_mtr_eth_policer_glbl_profile_map_set(
	 SOC_SAND_IN  int                  	unit,
	 SOC_SAND_IN  int                    core_id,
	 SOC_SAND_IN  SOC_PPC_PORT           port,
	 SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE   eth_type_ndx,
	 SOC_SAND_IN  uint32                 glbl_profile_idx
  );

/*********************************************************************
* NAME:
*     jer_pp_mtr_eth_policer_params_get
* FUNCTION:
*     gets the local Eth. policer on a specific port
* INPUT:
*   int                         	unit				- Identifier of the device to access
*   SOC_PPC_PORT                    port_ndx			- specify port index to meter
*	SOC_PPC_MTR_ETH_TYPE        	eth_type_ndx		- specify packet's type to meter
* OUTPUT:
*	SOC_PPC_MTR_BW_PROFILE_INFO		*policer_info		- specify Eth. policer parameters
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32  
  jer_pp_mtr_eth_policer_glbl_profile_map_get(
	 SOC_SAND_IN  int                  	unit,
	 SOC_SAND_IN  int                   core_id,
	 SOC_SAND_IN  SOC_PPC_PORT          port,
	 SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE  eth_type_ndx,
	 SOC_SAND_OUT uint32                *glbl_profile_idx
   );

/*********************************************************************
* NAME:
*     jer_pp_mtr_policer_ipg_compensation_set
* FUNCTION:
*     Enables the global IPG compensation
*     If TRUE then a constant value of 20 will be added to packet size for meter calculations.
*     If FALSE then nothing is added. 
* 	  This is relevant both for the ethernet policer and for the MRPS.
*
* INPUT:
*   int                         unit
*   	- Identifier of the device to access
*   uint8						ipg_compensation_enabled
*   	- Should global compensation be enabled or not
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
uint32
jer_pp_mtr_policer_ipg_compensation_set(
    int                         unit,
	uint8						ipg_compensation_enabled
);

uint32
jer_pp_mtr_policer_ipg_compensation_get(
    int                         unit,
	uint8						*ipg_compensation_enabled
);

/*********************************************************************
* NAME:
*     jer_pp_mtr_policer_hdr_compensation_set
* FUNCTION:
*     Get header compensation value per pp port.
*     Up to 7 profiles (beside of the default profile) are supported.
*
* INPUT:
*   int                         unit
*   	- Identifier of the device to access
*   int                         core_id
*       - Identifier of the device core
*   uint32						compensation
*   	- Compensation size in bytes
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called from the mbcm_pp dispatcher.
*********************************************************************/
uint32
jer_pp_mtr_policer_hdr_compensation_set(
    int                         unit,
	int                         core_id,
	uint32                      pp_port,
	int                         compensation_size
);

uint32
jer_pp_mtr_policer_hdr_compensation_get(
    int                         unit,
	int                         core_id,
	uint32                      pp_port,
	int                         *compensation_size
);
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __JER_PP_METERING_INCLUDED__*/
#endif

