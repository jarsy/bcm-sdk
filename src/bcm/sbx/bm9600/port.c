/*
 * $Id: port.c,v 1.102 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 Port API
 */

#include <shared/bsl.h>

#include <sal/core/time.h>
#include <soc/portmode.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/link.h>
#include <bcm/stack.h>

#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>

static uint32 uPrbsModeSi[SOC_MAX_NUM_DEVICES];
static uint32 uPrbsForceTxError[SOC_MAX_NUM_DEVICES][BM9600_NUM_LINKS];
static uint32 uDriverEqualizationFarEnd[SOC_MAX_NUM_DEVICES][BM9600_NUM_LINKS];

#define BM9600_PORT_CHECK(unit, port)           \
  do {                                          \
      if ((port >= BM9600_NUM_LINKS)   ||       \
          (!SOC_PBMP_PORT_VALID(port)) ||       \
          (!(IS_SFI_PORT(unit, port)   ||       \
             IS_SCI_PORT(unit, port)))) {       \
            return BCM_E_PARAM;                 \
      }                                         \
  } while(0)

int
bcm_bm9600_port_speed_get(int unit, bcm_port_t port, int *speed);

int
bcm_bm9600_ability_matching_speed_set(int unit, bcm_port_t port, int ability)
{    
    uint32 speed;
    int32 encoding;
    int rv = BCM_E_NONE;
    int32 node;

    /* update configuration */
    switch (ability) {
        case BCM_PORT_ABILITY_DUAL_SFI:
        case BCM_PORT_ABILITY_SFI_SCI:
        case BCM_PORT_ABILITY_DUAL_SFI_LOCAL:
	    speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
	    encoding = SOC_SBX_CFG(unit)->bSerdesEncoding;
	    break;

	case BCM_PORT_ABILITY_SCI:

	    if ((port < 0) || (port >= BCM_STK_MAX_MODULES)) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ability_matching_speed_set: unit %d node %d out of range\n"),
                           unit, port));
		return BCM_E_PARAM;
	    }

	    /* Get the node from the physical SCI pin number */
	    node = SOC_SBX_P2L_NODE(unit, port);

	    if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocolNone)) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ability_matching_speed_set: unit %d must set module protocol for destination node\n"),
                           unit));
		return BCM_E_PARAM;
	    }

	    if ((SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol3) ||
		(SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol4))  {

		speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
		encoding = SOC_SBX_CFG(unit)->bSerdesEncoding;
	    } else {

		speed = 3125;
		encoding = TRUE;

	    }
	    break;


        case BCM_PORT_ABILITY_SFI:
	    speed = 3125;
	    encoding = TRUE;
	    break;

	default:
            return(BCM_E_PARAM);
    }
#if 000
    LOG_CLI((BSL_META_U(unit,
                        "port(%d) speed(%d) encoding(%s)\n"), port, speed, encoding?"8b10b":"64b66b"));
#endif

    /* Set speed */
    SOC_IF_ERROR_RETURN(soc_bm9600_hc_speed_set(unit, port, speed));

    /* Set encoding */
    SOC_IF_ERROR_RETURN(soc_bm9600_hc_encoding_set(unit, port, encoding));

    /* Set drive strength and equalization */
    SOC_IF_ERROR_RETURN(soc_bm9600_config_linkdriver(unit, port));

    return rv;
}
STATIC int
_bcm_bm9600_port_prbs_rx_setup(int unit, bcm_port_t port, int bEnablePrbs)
{
    int nLink = port;
    uint32 uData;
    int port_speed;
    
    SOC_IF_ERROR_RETURN(bcm_bm9600_port_speed_get(unit, port, &port_speed));
    
    if (uPrbsModeSi[unit]) {
	
	uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, ENABLE, uData, bEnablePrbs? 0:1);
	SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	
	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed == 3125)) { 
	    
	    /* Byte swapping is enabled if we are running w 8b10b, need to disable to run prbs */
	    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, RX_BYTE_SWAP, uData, bEnablePrbs?0:1);
	    SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	    
	    if (port_speed == 6250) {
		/* need to disable scrambler also */
		SOC_IF_ERROR_RETURN
		    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_SCRAMBLER,bEnablePrbs?0:1));
		
	    }
	}
	if ((port_speed > 3125) && (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {
	    /* Disable or enable 64/66 encoding */
	    SOC_IF_ERROR_RETURN
		(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_64B66B, bEnablePrbs?0:1));
	}
	
	/* If we are enabling PRBS, we need to disable 8b10b encoding and comma detect */
	/* in the hypercore.  Otherwise, we need to enable 8b10b encoding and comma    */
	/* detect in the hypercore - for all encodings/speeds, this is set for normal operation */
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_8B10B, bEnablePrbs?0:1));
	
	
	
	uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, PRBS_MONITOR_ENABLE, uData, bEnablePrbs? 1:0);
	SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	
	/* Read to clear prbs_err_cnt */
	SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_PRBS_STATUS);
    }
    /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
    /* But, on the receive side, set the status select so that the receive status is set up for PRBS */
    else {
	
	/* GNATS 19203, when enabling/disabling PRBS if 3.125G, must disable 8b10b swapping */
	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed==3125)) {
	    
	    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, RX_BYTE_SWAP, uData, (bEnablePrbs)?0:1);
	    SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	} 
    
	/* Disable 8b10b/comma detect or re-enable it */
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_8B10B, (bEnablePrbs)?0:1));
	
	if ((port_speed > 3125) && (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {
	    
	    /* Disable or enable 64/66 encoding */
	    SOC_IF_ERROR_RETURN
		(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_64B66B, bEnablePrbs?0:1));
	}
	
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, 
				     SOC_PHY_CONTROL_PRBS_RX_ENABLE, 
				     bEnablePrbs));
	
	/* Read to clear initial status */
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_get(unit, port, 
				     SOC_PHY_CONTROL_PRBS_RX_STATUS,
				     &uData));	
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_bm9600_port_prbs_tx_setup(int unit, bcm_port_t port, int bEnablePrbs) 
{

    int nLink = port;
    uint32 uData;
    int port_speed;

    SOC_IF_ERROR_RETURN(bcm_bm9600_port_speed_get(unit, port, &port_speed));

    /* SI PRBS */
    if (uPrbsModeSi[unit]) {

	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed == 3125)) { 
	    
	    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, TX_BYTE_SWAP, uData, (bEnablePrbs)?0:1);
	    SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	    
	    if (port_speed == 6250) {
		
		SOC_IF_ERROR_RETURN
		    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_SCRAMBLER,
					     (bEnablePrbs)?0:1));
	    }
	}

	/* If we are enabling PRBS, we need to disable 8b10b encoding and comma detect */
	/* in the hypercore.  Otherwise, we need to enable 8b10b encoding and comma    */
	/* detect in the hypercore                                                     */
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_8B10B,
				     (bEnablePrbs)?0:1));

	if ((port_speed > 3125) && (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {

	    /* Disable 64/66 encoding */
	    SOC_IF_ERROR_RETURN
		(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_64B66B, bEnablePrbs?0:1));

	}

	uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, PRBS_GENERATOR_ENABLE, uData, (bEnablePrbs)?1:0);
	SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	
    }
    /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
    else {
	
	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed==3125)) {
	    
	    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
	    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, TX_BYTE_SWAP, uData, (bEnablePrbs)?0:1);
	    SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);	    	    
	}

	/* If we are enabling PRBS, we need to disable 8b10b encoding and comma detect */
	/* in the hypercore.  Otherwise, we need to enable 8b10b encoding and comma    */
	/* detect in the hypercore                                                     */
	/* Disable 8b10b/comma detect or re-enable it */
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_8B10B,
				     (bEnablePrbs)?0:1));

	if ((port_speed > 3125) && (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {

	    /* Disable 64/66 encoding */
	    SOC_IF_ERROR_RETURN
		(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_64B66B, bEnablePrbs?0:1));

	}

	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, 
				     SOC_PHY_CONTROL_PRBS_TX_ENABLE, 
				     bEnablePrbs));
    }
    return BCM_E_NONE;
}


STATIC int
_bcm_bm9600_port_probe(int unit, bcm_port_t port, int *okay)
{
    int           rv;

    *okay = FALSE;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Init port %d PHY...\n"), port));
    if ((rv = soc_phyctrl_probe(unit, port)) < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d Port %s: Failed to probe PHY: %s\n"),
                  unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }

    if (!SOC_WARM_BOOT(unit) && 
        ((rv = soc_phyctrl_init(unit, port)) < 0)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d Port %s: Failed to initialize PHY: %s\n"),
                  unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }

    /* enconding == 0 => 64b66b */
    if ((SOC_SBX_CFG(unit)->uSerdesSpeed == 3125) &&
	(SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Cannot set the speed to 3.125 with 64b66b encoding "
                             "(bSerdesEncoding=0)\n")));
        rv = SOC_E_PARAM;
        return rv;
    }

    SOC_IF_ERROR_RETURN
	(soc_bm9600_hc_speed_set(unit, port, SOC_SBX_CFG(unit)->uSerdesSpeed));

    SOC_IF_ERROR_RETURN
	(soc_bm9600_hc_encoding_set(unit, port, SOC_SBX_CFG(unit)->bSerdesEncoding));

    *okay = TRUE;

    return SOC_E_NONE;
}

int
bcm_bm9600_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int rv = BCM_E_NONE;
    bcm_port_t port;
    int okay;
    uint32 uSiConfig0;
    uint32 uSiConfig3;
    uint32 uSiSdConfig;

    SOC_PBMP_CLEAR(*okay_pbmp);

    PBMP_ITER(pbmp, port) {
        rv = _bcm_bm9600_port_probe(unit, port, &okay);
        if (okay) {
            SOC_PBMP_PORT_ADD(*okay_pbmp, port);
        }
        if (rv < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Port probe failed on port %s: %s\n"),
                      SOC_PORT_NAME(unit, port), 
                      bcm_errmsg(rv)));
            break;
        }
    }

    /* hwBm9600InitSiStep2 moved here */
    for ( port = 0; port < BM9600_NUMBER_OF_SI; port++ ) {

	uSiConfig0 = SAND_HAL_READ_STRIDE(unit, PL, SI, port, SI_CONFIG0);
        uSiSdConfig = SAND_HAL_READ_STRIDE(unit, PL, SI, port, SI_SD_CONFIG);
        COMPILER_REFERENCE(uSiSdConfig);
	
	if (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_LCM_MODE) {
	    uSiConfig0 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG0, FORCE_TIME_ALIGNMENT_EVEN, uSiConfig0, 1);
	    uSiConfig0 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG0, FORCE_TIME_ALIGNMENT_ODD, uSiConfig0, 1);
	}
	
	/* setting JITTER TOLERANCE is not part of the initialization spec */
	uSiConfig3 = SAND_HAL_READ_STRIDE(unit, PL, SI, port, SI_CONFIG3);

	/* set jitter tolerance */
	
	if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
	    if (SOC_SBX_CFG(unit)->uSerdesSpeed == 6500) {
		uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, JIT_TOLERANCE, uSiConfig3, 20);
	    } else {
		uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, JIT_TOLERANCE, uSiConfig3, 24);
	    }
	} else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
            uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, JIT_TOLERANCE, uSiConfig3, 24);
	} else {
	    uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, JIT_TOLERANCE, uSiConfig3, 16);
	}

	/* If both channels are on, double channel, else single channel */
	
	uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, CH_MODE, uSiConfig3, 0);
	
	/* Steer even channels based on init param for channel type.  Odd channels are always XB directed */
	if( soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
	    uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, EVEN_CH_DATA_SELECT, uSiConfig3, 0);
	} 
	/* Handles sfi_sci or sfi only links */
	if (soc_property_port_get(unit, port, spn_PORT_IS_SFI, 0) ){
	    uSiConfig3 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG3, EVEN_CH_DATA_SELECT, uSiConfig3, 1);
	}
		
	SAND_HAL_WRITE_STRIDE(unit, PL, SI, port, SI_CONFIG3, uSiConfig3);
	SAND_HAL_WRITE_STRIDE(unit, PL, SI, port, SI_CONFIG0, uSiConfig0);
    }

    soc_bm9600_config_all_linkdriver(unit);

    return rv;
}

int
bcm_bm9600_port_interface_init(int unit)
{
    int                 rv = BCM_E_NONE;
    bcm_port_t          p, port_enable;
    pbmp_t              okay_ports;
    pbmp_t              all_ports;
    char                pfmtok[SOC_PBMP_FMT_LEN],
                        pfmtall[SOC_PBMP_FMT_LEN];

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "bcm_port_init: unit %d\n"), unit));

    if (unit >= BCM_MAX_NUM_UNITS) {
        rv = BCM_E_PARAM;
        return(rv);
    }

    /* alloc memory for port info, alloc semophore for port */
    if ((SOC_SBX_STATE(unit)->port_state->port_lock = sal_mutex_create("PORT")) 
        == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "port init: unit %d unable to create port lock\n"),
                   unit));
        rv = BCM_E_MEMORY;
        goto error;
    }

    if ((rv = soc_phy_common_init(unit)) != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Error unit %d:  Failed common PHY driver init: %s\n"),
                   unit, bcm_errmsg(rv)));
        goto error;
    }

    /* Probe for ports */
    BCM_PBMP_ASSIGN(all_ports, PBMP_PORT_ALL(unit));

    SOC_PBMP_CLEAR(okay_ports);
    if ((rv = bcm_port_probe(unit, all_ports, &okay_ports)) !=
        BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Error unit %d:  Failed port probe: %s\n"),
                   unit, bcm_errmsg(rv)));
        goto error;
    }

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Probed ports okay: %s of %s\n"),
                 SOC_PBMP_FMT(okay_ports, pfmtok),
                 SOC_PBMP_FMT(all_ports, pfmtall)));

    PBMP_ITER(okay_ports, p) {
	/*
	 * disable ports
	 * when switch boots up
	 */
#undef BCM_PORT_DEFAULT_DISABLE
#define BCM_PORT_DEFAULT_DISABLE
#ifdef BCM_PORT_DEFAULT_DISABLE
	port_enable = FALSE;
#else
	port_enable = TRUE;
#endif  /* BCM_PORT_DEFAULT_DISABLE */
	if ((rv = bcm_port_enable_set(unit, p, port_enable)) < 0) {
            /* coverity [dead_error_line] */
	    LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to %s port: %s\n"),
                      unit, SOC_PORT_NAME(unit, p),(port_enable) ? "enable" : "disable" ,bcm_errmsg(rv)));
	}
    }
    return rv;

error:
    if (SOC_SBX_STATE(unit)->port_state->port_lock != NULL) {
        sal_mutex_destroy(SOC_SBX_STATE(unit)->port_state->port_lock);
        SOC_SBX_STATE(unit)->port_state->port_lock = NULL;
    }

    return rv;
}



int
bcm_bm9600_port_init(int unit)
{
  int nDevice, nPort, eg, fifo = 0;
    int rv;
    bcm_port_t          p;

    for (nDevice = 0; nDevice < SOC_MAX_NUM_DEVICES; nDevice++) {
	for (nPort =0; nPort < SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; nPort++) {
	    uDriverEqualizationFarEnd[nDevice][nPort] = -1;
	}
    }
    
    rv = bcm_bm9600_port_interface_init(unit);

    if ((SOC_SBX_STATE(unit)->port_state->subport_info == NULL) &&
	((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
	 (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE))) {
        SOC_SBX_STATE(unit)->port_state->subport_info = sal_alloc(sizeof(bcm_sbx_subport_info_t)
								  * SOC_SBX_CFG(unit)->cfg_num_nodes
                                                                  *  SB_FAB_DEVICE_MAX_FABRIC_PORTS,
                                                                  "subport_info");
        if (SOC_SBX_STATE(unit)->port_state->subport_info == NULL) {
            rv = BCM_E_MEMORY;
            goto error;
        }
        /* Initialize structure to 0s, then make only needed changes */
        sal_memset(SOC_SBX_STATE(unit)->port_state->subport_info, 0,
                   sizeof(bcm_sbx_subport_info_t) *  SB_FAB_DEVICE_MAX_FABRIC_PORTS);
	for (p = 0; p <  (SOC_SBX_CFG(unit)->cfg_num_nodes * SB_FAB_DEVICE_MAX_FABRIC_PORTS); p++) {
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].valid = FALSE;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].parent_gport = BCM_GPORT_INVALID;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].original_gport = BCM_GPORT_INVALID;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].group_shaper = -1;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].port_offset = -1;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].ts_scheduler_level = -1;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].ts_scheduler_node = -1;
	    SOC_SBX_STATE(unit)->port_state->subport_info[p].es_scheduler_level2_node = -1;
	    for (eg=0; eg < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; eg++) {
		SOC_SBX_STATE(unit)->port_state->subport_info[p].es_scheduler_level1_node[eg] = -1;
		SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].es_scheduler_level0_node = -1;
		SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].egroup_gport = -1;
		for (fifo=0; fifo <  SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; fifo++) {
		  SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].fcd[fifo] = -1;
		}
	    }
	}
    }
    SOC_SBX_STATE(unit)->port_state->fabric_header_format = BCM_PORT_CONTROL_FABRIC_HEADER_88230;

    return rv;

 error:
    if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
        sal_free(SOC_SBX_STATE(unit)->port_state->subport_info);
	SOC_SBX_STATE(unit)->port_state->subport_info = NULL;
    }
    return rv;
}

int
bcm_bm9600_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm9600_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int rv = BCM_E_UNAVAIL;
    int nLink = port;
    uint32 uData;

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
	rv = BCM_E_NONE;
	switch (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port]) {
	    case BCM_PORT_ABILITY_DUAL_SFI:
	    case BCM_PORT_ABILITY_DUAL_SFI_LOCAL:
	    case BCM_PORT_ABILITY_SFI_SCI:
		*speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
		break;
	    case BCM_PORT_ABILITY_SFI:
		*speed = 3125;
		break;
	    case BCM_PORT_ABILITY_SCI:
		/* If dual channel, we cannot be running at 3125G */
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
		uData = SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, CH_MODE, uData);
		if (uData) /* single channel */ {
		    *speed = 3125;
		} else     /* dual channel   */ {
		    *speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
		}
		break;
	    default:
		rv = BCM_E_PARAM;
	}
    }

    return rv;
}
int
bcm_bm9600_port_link_get(int unit, bcm_port_t port, int *up)
{
    uint32 status = 0;
    int    timeEvenAligned     = FALSE;
    int    timeOddAligned      = FALSE;
    int    byteAligned         = FALSE;
    uint32 ch_config;

    soc_persist_t *sop = SOC_PERSIST(unit);

    BM9600_PORT_CHECK(unit, port);

    /* mark link as down */
    (*up) = FALSE;

    /* SDK-30949 */
    /* If the state indicates that we are not byte aligned regardless of sticky state,  */
    /* report link as down.  This is because the sticky state might be 0 if the link is */
    /* down and has been down continuously after being checked and cleared.             */
    status = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, port, SI0_STATE);
    if ( (SAND_HAL_GET_FIELD(PL, SI0_STATE, MSM_RUN_BYTE_ALIGNMENT, status) == 1) ) {
	*up = FALSE;
	return(BCM_E_NONE);
    }

    status = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, port, SI0_STICKY_STATE);

    /* SDK-30949 */
    /* If the sticky state is 0, there is no change from the previous time we have checked */
    /* Note that the user should not check the sticky state independendently as this will  */
    /* interfere with port status collection.                                              */
    if (status == 0) {
	*up = (SOC_PBMP_MEMBER(sop->lc_pbm_link, port)) ?
	    BCM_PORT_LINK_STATUS_UP : BCM_PORT_LINK_STATUS_DOWN;
	return(BCM_E_NONE);
    }

    /* check Byte alignment */
    if ( (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_MSM_RUN_BYTE_ALIGNMENT, status) == 1) ||
	 (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_MSM_LOST_BYTE_ALIGNMENT, status) == 1) ) {
	byteAligned = FALSE;
    } else {
	byteAligned = TRUE; /* no other indicator of byte alignment */
    }

    /* check Even Time alignment */
    if ( (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_EVEN_IDLE, status) != 1) &&
            (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_EVEN_SOT_SEARCH, status) != 1) &&
            (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_EVEN_SOT_EARLY, status) != 1) &&
            (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_EVEN_SOT_MISSING, status) != 1) ) {
        timeEvenAligned = TRUE;
    }

    /* check Odd Time alignment */
    ch_config = SAND_HAL_READ_STRIDE(unit, PL, SI, port, SI0_CONFIG3);

    /* If single channel or odd channel in loopback, don't check odd channel status */
    if ( (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, CH_MODE, ch_config) == 1)                      ||
	 (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] == BCM_PORT_ABILITY_DUAL_SFI_LOCAL)
       )
    {
        timeOddAligned = TRUE;
    }
    else if ( (port < BCM_STK_MAX_MODULES) &&
	      (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] == BCM_PORT_ABILITY_SFI_SCI) &&
	      (SOC_SBX_STATE(unit)->stack_state->protocol[port] == bcmModuleProtocol4)
	    ) {
        timeOddAligned = TRUE;
    }
    else {
	if ( (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_ODD_IDLE, status) != 1) &&
	     (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_ODD_SOT_SEARCH, status) != 1) &&
	     (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_ODD_SOT_EARLY, status) != 1) &&
	     (SAND_HAL_GET_FIELD(PL, SI0_STICKY_STATE, S_TASM_ODD_SOT_MISSING, status) != 1) ) {
	    timeOddAligned = TRUE;
        }
    }

    /* Clear sticky state for next read */
    SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, port, SI0_STICKY_STATE, status);


    if (timeEvenAligned != TRUE) {
	SOC_PBMP_PORT_REMOVE(sop->lc_pbm_link, port);
        return(BCM_E_NONE);
    }
    if (byteAligned != TRUE) {
	SOC_PBMP_PORT_REMOVE(sop->lc_pbm_link, port);
        return(BCM_E_NONE);
    }
    if (timeOddAligned != TRUE) {
	SOC_PBMP_PORT_REMOVE(sop->lc_pbm_link, port);
        return(BCM_E_NONE);
    }

    /* mark link as up */
    (*up) = TRUE;
    SOC_PBMP_PORT_ADD(sop->lc_pbm_link, port);

    return(BCM_E_NONE);
}

int
bcm_bm9600_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    soc_persist_t *sop = SOC_PERSIST(unit);

    BM9600_PORT_CHECK(unit, port);

    /* If linkscan maintains the state, return the saved state, otherwise read the hw state */
    if (bcm_linkscan_enable_port_get(unit, port) == BCM_E_DISABLED) {

	return bcm_bm9600_port_link_get(unit, port, up);

    } else {
	*up = (SOC_PBMP_MEMBER(sop->lc_pbm_link, port)) ?
	    BCM_PORT_LINK_STATUS_UP : BCM_PORT_LINK_STATUS_DOWN;

    }
    return(BCM_E_NONE);
}

/* This routine sets up local loopback tx to rx at the Hypercore digital interface */
int
bcm_bm9600_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    phy_ctrl_t   *int_pc;
    phy_driver_t *pd;

    INT_PHY_INIT_CHECK(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    pd = int_pc->pd;
    return PHY_LOOPBACK_SET(pd, unit, port, loopback != BCM_PORT_LOOPBACK_NONE);
}

int
bcm_bm9600_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{ 
    phy_ctrl_t   *int_pc;
    phy_driver_t *pd;

    INT_PHY_INIT_CHECK(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    pd = int_pc->pd;
    return PHY_LOOPBACK_GET(pd, unit, port, loopback);
}


#define BM9600_XB_IPORT_ENABLE_INDEX_DIVISOR  (32/2 /* bits per iport register/2 channels per link (even/odd) */)

static int
_bcm_bm9600_xb_iport_enable_set(int unit, int32_t nLink, int nAbility, int nEnable)
{
    uint32 uIprtEnableIndex;
    uint32 uIprtEnable;
    int rv = BCM_E_NONE;

    /* There are 2 channels per link, even and odd */
    uIprtEnableIndex = nLink/BM9600_XB_IPORT_ENABLE_INDEX_DIVISOR;
    switch (uIprtEnableIndex) {
	case 0:
	    uIprtEnable = SAND_HAL_READ((sbhandle)unit, PL, XB_IPRT_ENABLE0);
	    break;
	case 1:
	    uIprtEnable = SAND_HAL_READ((sbhandle)unit, PL, XB_IPRT_ENABLE1);
	    break;
	case 2:
	    uIprtEnable = SAND_HAL_READ((sbhandle)unit, PL, XB_IPRT_ENABLE2);
	    break;
	case 3:
	    uIprtEnable = SAND_HAL_READ((sbhandle)unit, PL, XB_IPRT_ENABLE3);
	    break;
	case 4:
	    uIprtEnable = SAND_HAL_READ((sbhandle)unit, PL, XB_IPRT_ENABLE4);
	    break;
	case 5:
	    uIprtEnable = SAND_HAL_READ((sbhandle)unit, PL, XB_IPRT_ENABLE5);
	    break;
	default:
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "Invalid xb_iprt_enable index(%d) must be less than 6\n"),
	               uIprtEnableIndex));
	    return BCM_E_PARAM;
    }

    /* Disable both even and odd */
    uIprtEnable &= ~(3 << ((nLink % BM9600_XB_IPORT_ENABLE_INDEX_DIVISOR) * 2));

    if (nEnable) {
        switch (nAbility) {
        case BCM_PORT_ABILITY_SFI:
            /* Enable only the even */
            uIprtEnable |= 1 << ((nLink % BM9600_XB_IPORT_ENABLE_INDEX_DIVISOR) * 2);
#if 000

	    LOG_CLI((BSL_META_U(unit,
                                "even channel only nLink(%d) uIprtEnable(0x%08x)\n"), nLink, uIprtEnable));
#endif
            break;
        case BCM_PORT_ABILITY_DUAL_SFI:
            /* Enable both even and odd */
            uIprtEnable |= 3 << ((nLink % BM9600_XB_IPORT_ENABLE_INDEX_DIVISOR) * 2);
#if 000

	    LOG_CLI((BSL_META_U(unit,
                                "even and odd channels nLink(%d) uIprtEnable(0x%08x)\n"), nLink, uIprtEnable));
#endif
            break;
        case BCM_PORT_ABILITY_SFI_SCI:
            /* Enable only the odd */
            uIprtEnable |= 2 << ((nLink % BM9600_XB_IPORT_ENABLE_INDEX_DIVISOR) * 2);
            break;
        case BCM_PORT_ABILITY_SCI:
            /* No need to enable iport in this case, just return  */
            return rv;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Invalid port ability(%d)\n"),
                       nAbility));
            return rv;
        }
    }
    switch (uIprtEnableIndex) {
	case 0:
	    SAND_HAL_WRITE((sbhandle)unit, PL, XB_IPRT_ENABLE0, uIprtEnable);
	    break;
	case 1:
	    SAND_HAL_WRITE((sbhandle)unit, PL, XB_IPRT_ENABLE1, uIprtEnable);
	    break;
	case 2:
	    SAND_HAL_WRITE((sbhandle)unit, PL, XB_IPRT_ENABLE2, uIprtEnable);
	    break;
	case 3:
	    SAND_HAL_WRITE((sbhandle)unit, PL, XB_IPRT_ENABLE3, uIprtEnable);
	    break;
	case 4:
	    SAND_HAL_WRITE((sbhandle)unit, PL, XB_IPRT_ENABLE4, uIprtEnable);
	    break;
	case 5:
	    SAND_HAL_WRITE((sbhandle)unit, PL, XB_IPRT_ENABLE5, uIprtEnable);
	    break;
    }
    return rv;
}

int
bcm_bm9600_port_control_set(int unit, bcm_port_t port,
                            bcm_port_control_t type, int value)
{
    uint32 uData = 0;
    int nLink;
    soc_info_t *si;
    int status = BCM_E_NONE;
    int enable = FALSE;

    if (BCM_GPORT_IS_MODPORT(port)) {
        port = BCM_GPORT_MODPORT_PORT_GET(port);
    }

    BM9600_PORT_CHECK(unit, port);

    si = &SOC_INFO(unit);
    nLink = port;
    switch (type) {
        case bcmPortControlPrbsMode:
	    if (value == 0) {
		/* PRBS in phy - lower layer */
		uPrbsModeSi[unit] = 0;
	    }
	    else {
		/* PRBS in SI block */
		uPrbsModeSi[unit] = 1;
	    }
	    break;
        case bcmPortControlPrbsPolynomial:
	    /* PRBS in SI */
	    if (uPrbsModeSi[unit]) {
		if ((value != BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1) &&
		    (value != BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1)) {
		    return BCM_E_PARAM;
		}
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
		uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, PRBS_POLY_SELECT, uData, value);
		SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	    }
	    else {
                switch (value) {
                case (BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1):
                    value = 0;
                    break;
                case (BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1):
                    value = 1;
                    break;
                case (BCM_PORT_PRBS_POLYNOMIAL_X23_X18_1):
                    value = 2;
                    break;
                case (BCM_PORT_PRBS_POLYNOMIAL_X31_X28_1):
                    value = 3;
                    break;
                default:
                    return BCM_E_PARAM;
                    break;
		}
                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_set(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_POLYNOMIAL, 
                                             value));

	    }
            break;
        case bcmPortControlPrbsTxInvertData:
	    /* SI PRBS */
	    if (uPrbsModeSi[unit]) {
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
		uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG0, PRBS_INVERT, uData, (value)?1:0);
		SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, uData);
	    }
	    /* Hypercore PRBS */
	    else {
                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_set(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA, 
                                             value));
	    }
            break;
        case bcmPortControlPrbsForceTxError:
	    if (uPrbsModeSi[unit]) {
		/* Read the SI0_PRBS_STATUS register will clear the error count.
		   We retrieve the state from the stored software state. This is
		   Assuming all other fields are read only
		*/
		uData = 0;
		uData = SAND_HAL_MOD_FIELD(PL, SI0_PRBS_STATUS, FORCE_ERROR, uData, (value)?1:0);
		SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_PRBS_STATUS, uData);
		/* store the force tx state */
		uPrbsForceTxError[unit][nLink] = (value)?1:0;
	    }
	    else {


		/* Not supported in Hypercore */
		return BCM_E_UNAVAIL;
	    }
            break;

        case bcmPortControlPrbsTxEnable:
	    status = _bcm_bm9600_port_prbs_tx_setup(unit, port, value);
            break;

        case bcmPortControlPrbsRxEnable:
	    status = _bcm_bm9600_port_prbs_rx_setup(unit, port, value);
            break;

        case bcmPortControlSerdesDriverStrength:
            if ( (value > LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX) || (value < 0)) {
                return BCM_E_PARAM;
            }
            SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[nLink].uDriverStrength = value;
            soc_bm9600_config_linkdriver(unit, nLink);
            break;
        case bcmPortControlSerdesDriverEqualization:
            if ( (value > LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX) || (value < 0)) {
                return BCM_E_PARAM;
            }
	    SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[nLink].uDriverEqualization = value;
	    soc_bm9600_config_linkdriver(unit, nLink);
            break;
        case bcmPortControlSerdesDriverEqualizationFarEnd:
            if ( (value > LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX) || (value < 0)) {
                return BCM_E_PARAM;
            }
            uDriverEqualizationFarEnd[unit][port] = value;
            break;
        case bcmPortControlSerdesDriverTune:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_control_set(unit, port, 
                                         SOC_PHY_CONTROL_SERDES_DRIVER_TUNE,
                                         value));
            break;
        case bcmPortControlAbility:

            if (value == BCM_PORT_ABILITY_SFI) {

                /* Single channel SFI */
            /* Read to get current value of config3 */
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, CH_MODE, uData, 1 /* single-channel */);
                uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT,
					   uData, 1 /* send data to crossbar */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3, uData);
		SAND_HAL_RMW_FIELD_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, L2R_TX_LOS_SELECT, 0);

		SBX_ADD_PORT(sfi, nLink);
		SBX_REMOVE_PORT(sci, nLink);
		sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
			     "sfi%d", port);
            }
            else if ((value == BCM_PORT_ABILITY_DUAL_SFI) ||
	             (value == BCM_PORT_ABILITY_DUAL_SFI_LOCAL)) {


		/* Read to get current value of config3 */
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
		if (SOC_SBX_CFG(unit)->uSerdesSpeed > 3125) {
		    /* Dual channel on sirius if speed greater than 3125 */
		    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, CH_MODE, uData, 0);
		} else {
		    /* Single channel SCI */
		    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, CH_MODE, uData, 1);
		    
		}

                uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT,
					   uData, 1 /* send even data to crossbar */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3, uData);

		SAND_HAL_RMW_FIELD_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, L2R_TX_LOS_SELECT, 1);

		SBX_ADD_PORT(sfi, nLink);
		SBX_REMOVE_PORT(sci, nLink);
		sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
			     "sfi%d", port);
            }
            else if (value == BCM_PORT_ABILITY_SCI) {

         	uData = SAND_HAL_READ_STRIDE(unit, PL, AI, nLink /* physical node */, AI_CONFIG);
	        uData = SAND_HAL_GET_FIELD(PL, AI_CONFIG, QE40_MODE, uData);

		/* If protocol 5 (3.125G and interfacing with QE40 protocol) */
		if ((SOC_SBX_CFG(unit)->uSerdesSpeed > 3125) && (uData)){
		    /* Dual channel SFI_SCI to allow higher speed link */
		    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
		    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, CH_MODE, uData, 0 /* dual-channel */);
		}
		else {
		  /* Single channel SCI */
		    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
		    uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, CH_MODE, uData, 1 /* single-channel */);
		}

		uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT,
					   uData, 0 /* send data to arbiter */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3, uData);
		SAND_HAL_RMW_FIELD_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, L2R_TX_LOS_SELECT, 0);

		SBX_ADD_PORT(sci, nLink);
		SBX_REMOVE_PORT(sfi, nLink);
		sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
			     "sci%d", port);
            }
            else if (value == BCM_PORT_ABILITY_SFI_SCI) {

                /* Dual channel SFI/SCI */
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
                uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, CH_MODE, uData, 0 /* dual-channel */);
                uData = SAND_HAL_MOD_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT,
					   uData, 0 /* send even data to arbiter */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3, uData);
		SAND_HAL_RMW_FIELD_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0, L2R_TX_LOS_SELECT, 1);

		SBX_ADD_PORT(sci, nLink);
		SBX_ADD_PORT(sfi, nLink);
		sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
			     "sci_sfi%d", port);
            }
	    else {
		return BCM_E_PARAM;
	    }

	    /* Enable the iport if the port was enabled - odd or even or both */
	    soc_phyctrl_enable_get(unit, port, &enable);
	    if (enable) {
            status = _bcm_bm9600_xb_iport_enable_set(unit, nLink, value, enable);
		if (status) {
		    return status;
		}
	    }
	    /* Store new ability for the port - need this info when setting */
	    /* the drive strength.                                          */
	    SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] = value;

            status = bcm_bm9600_ability_matching_speed_set(unit, port, value);
            break;

        case bcmPortControlRxEnable:
	  if (value) {
	      SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_SD_CONFIG, RX_PWRDWN, 0);
	  }else {
	      SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_SD_CONFIG, RX_PWRDWN, 1);
	  }
	  break;

        case bcmPortControlTxEnable:
	  if (value) {
	      SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_SD_CONFIG, TX_PWRDWN, 0);
	      SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_SD_CONFIG, TX_FIFO_RESET, 0);
	  }else {
	      SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_SD_CONFIG, TX_PWRDWN, 1);
	      SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_SD_CONFIG, TX_FIFO_RESET, 1);
	  }
	  break;

        case bcmPortControlSerdesTuneMarginMode:
              status = soc_phyctrl_control_set(unit, port,
                                               SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE,
                                               value);
              break;
        case bcmPortControlSerdesTuneMarginValue:
            status = soc_phyctrl_control_set(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE,
                                             value);
            break;
        case bcmPortControlSerdesTuneMarginMax:
            status = soc_phyctrl_control_set(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MAX,
                                             value);
            break;
	case bcmPortControlFabricHeaderFormat:
	    switch (value) {
		case BCM_PORT_CONTROL_FABRIC_HEADER_83200:
		    break;
		case BCM_PORT_CONTROL_FABRIC_HEADER_88230:
		    break;
		case BCM_PORT_CONTROL_FABRIC_HEADER_88020_83200_88230_INTEROP:
		    break;
		case BCM_PORT_CONTROL_FABRIC_HEADER_CUSTOM:
		    break;
		default:
		    return BCM_E_UNAVAIL;
	    }
	    SOC_SBX_STATE(unit)->port_state->fabric_header_format = value;
 	    status = BCM_E_UNAVAIL;
	    break;
        default:
            return BCM_E_UNAVAIL;
    }

    return(status);
}

int
bcm_bm9600_port_control_get(int unit, bcm_port_t port,
                            bcm_port_control_t type, int *value)
{
    uint32 uData = 0;
    int nLink;
    int rv = BCM_E_NONE;

    if (type == bcmPortControlFabricHeaderFormat) {
	*value = SOC_SBX_STATE(unit)->port_state->fabric_header_format;
	rv = BCM_E_UNAVAIL;
	return rv;
    }

    BM9600_PORT_CHECK(unit, port);

    nLink = port;
    switch (type) {
        case bcmPortControlPrbsPolynomial:
	    if (uPrbsModeSi[unit]) {
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
		*value = SAND_HAL_GET_FIELD(PL, SI0_CONFIG0, PRBS_POLY_SELECT, uData);
	    } 
            else {
                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_get(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_POLYNOMIAL, 
                                             &uData));
		switch (uData) {
		    case 0:
			*value = BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1;
			break;
		    case 1:
			*value = BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1;
			break;			
		    case 2:
			*value = BCM_PORT_PRBS_POLYNOMIAL_X23_X18_1;
			break;
		    case 3:
			*value = BCM_PORT_PRBS_POLYNOMIAL_X31_X28_1;
			break;
		    default:
			*value = -1;
			break;
		}
	    }
            break;
        case bcmPortControlPrbsTxInvertData:
	    if (uPrbsModeSi[unit]) {
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
		*value = SAND_HAL_GET_FIELD(PL, SI0_CONFIG0, PRBS_INVERT, uData);
	    }
	    /* Hypercore PRBS */
	    else {
                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_get(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA,
                                             &uData));
                *value = uData;

	    }
            break;
        case bcmPortControlPrbsForceTxError:
	    if (uPrbsModeSi[unit]) {
		/* Read the SI0_PRBS_STATUS register will clear the error count.
		   We retrieve the state from the stored software state
		  uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_PRBS_STATUS);
		  *value = SAND_HAL_GET_FIELD(PL, SI0_PRBS_STATUS, FORCE_ERROR, uData);
		*/
		*value = uPrbsForceTxError[unit][nLink];
	    }
	    else {
		/* unavailable in hypercore */
		return BCM_E_UNAVAIL;
	    }
            break;
        case bcmPortControlPrbsTxEnable:
	    if (uPrbsModeSi[unit]) {
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
		*value = SAND_HAL_GET_FIELD(PL, SI0_CONFIG0, PRBS_GENERATOR_ENABLE, uData);
	    }
	    /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
	    else {

                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_get(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_TX_ENABLE,
                                             &uData));
                *value = uData;

	    }
            break;
        case bcmPortControlPrbsRxEnable:
	    /* SI PRBS */
	    if (uPrbsModeSi[unit]) {
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG0);
		*value = SAND_HAL_GET_FIELD(PL, SI0_CONFIG0, PRBS_MONITOR_ENABLE, uData);
	    }
	    /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
	    /* But, on the receive side, set the status select so that the receive status is set up for PRBS */
	    else {

                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_get(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_RX_ENABLE,
                                             &uData));
                *value = uData;

	    }
            break;
        case bcmPortControlPrbsRxStatus:
	    /* SI PRBS */
	    if (uPrbsModeSi[unit]) {
		uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_PRBS_STATUS);
		*value = SAND_HAL_GET_FIELD(PL, SI0_PRBS_STATUS, PRBS_ERR_CNT, uData);
	    }
	    /* Hypercore PRBS */
	    else {

                SOC_IF_ERROR_RETURN
                    (soc_phyctrl_control_get(unit, port, 
                                             SOC_PHY_CONTROL_PRBS_RX_STATUS,
                                             &uData));
                *value = uData;

	    }
	    break;
        case bcmPortControlSerdesDriverStrength:
            *value = SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[nLink].uDriverStrength;
            break;
        case bcmPortControlSerdesDriverEqualization:
            *value = SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[nLink].uDriverEqualization;
            break;
        case bcmPortControlSerdesDriverEqualizationTuneStatusFarEnd:

	    if (uDriverEqualizationFarEnd[unit][port] == -1) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "User has not set the bcmPortControlSerdesDriverEqualizationFarEnd value, please set the near end and the far end value of this field\n")));
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "Please start with 0 and increase the value based upon the information provided through tuning\n")));
		return BCM_E_PARAM;
	    }

                rv = (soc_phyctrl_control_get(unit, port, 
					      SOC_PHY_CONTROL_SERDES_DRIVER_EQUALIZATION_TUNE_STATUS_FAR_END,
					      &uData));

		if (rv) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "Error in tuning unit(%d) port(%d)\n"),
		               unit, port)); 
		} else {
		    
		    if (uData == FALSE) {
			
			if (uDriverEqualizationFarEnd[unit][port] < 15) {
			    
			    LOG_CLI((BSL_META_U(unit,
                                                "unit(%d) port(%d): Please increase far end postcursor value from (%d) to (%d) and retune\n"), 
                                     unit,
                                     port,
                                     uDriverEqualizationFarEnd[unit][port], 
                                     uDriverEqualizationFarEnd[unit][port] + 1));

			    *value = uData;
			    rv = BCM_E_FAIL;
			} 
		    }
		    else {
			LOG_CLI((BSL_META_U(unit,
                                            "unit(%d) port(%d): Far end postcursor value selection is good value(%d)\n"), 
                                 unit,
                                 port,
                                 uDriverEqualizationFarEnd[unit][port]));

			*value = uData;
			rv = BCM_E_NONE;
		    }
		}

            break;
        case bcmPortControlAbility:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PL, SI, nLink, SI0_CONFIG3);
	    if (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, CH_MODE, uData)) {
		/* This is a single-channel port. */
		if (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT, uData)) {
		    *value = BCM_PORT_ABILITY_SFI;
		}
		else {
		    *value = BCM_PORT_ABILITY_SCI;
		}
	    }
	    else {
		/* This is a dual-channel port. */
		if (SAND_HAL_GET_FIELD(PL, SI0_CONFIG3, EVEN_CH_DATA_SELECT, uData)) {
		    *value = BCM_PORT_ABILITY_DUAL_SFI;
		}
		else {
		  if (IS_SFI_PORT(unit, port) && IS_SCI_PORT(unit, port)) {
		    *value = BCM_PORT_ABILITY_SFI_SCI;
		  } else {
		  /* since we set the channel to dual channel mode     */
		  /* always for protocol3/4 on AI links, this channel  */
		  /* is really SCI                                     */
		    *value = BCM_PORT_ABILITY_SCI;
		  }
		}
	    }
	    break;

        case bcmPortControlRxEnable:
	    uData = SAND_HAL_READ_STRIDE(unit, PL, SI, port, SI_SD_CONFIG);
	    if (SAND_HAL_GET_FIELD(PL, SI_SD_CONFIG, RX_PWRDWN, uData)) {
		*value = 0;
	    } else {
		*value = 1;
	    }
	    break;

        case bcmPortControlTxEnable:
	    uData = SAND_HAL_READ_STRIDE(unit, PL, SI, port, SI_SD_CONFIG);
	    if (SAND_HAL_GET_FIELD(PL, SI_SD_CONFIG, TX_PWRDWN, uData)) {
		*value = 0;
	    } else {
		*value = 1;
	    }
	    break;
        case bcmPortControlSerdesTuneMarginMode:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_control_get(unit, port,
                                         SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE,
                                         (uint32 *)value));
            break;
        case bcmPortControlSerdesTuneMarginValue:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_control_get(unit, port,
                                         SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE,
                                         (uint32 *)value));
            break;
        case bcmPortControlSerdesTuneMarginMax:

            SOC_IF_ERROR_RETURN
                (soc_phyctrl_control_get(unit, port,
                                         SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MAX,
                                         (uint32 *)value));
            break;

        default:
            return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

/* New mbcm interface */
int
bcm_bm9600_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int32 nAbility;
    int rv = BCM_E_NONE;
    static int bFirstEnable = TRUE;
#ifndef _BM9600_INA_ENABLE_ON_
    int ina_config, ina_blk;
#endif /* _BM9600_INA_ENABLE_ON_ */

    BM9600_PORT_CHECK(unit, port);

    rv = bcm_bm9600_port_control_get(unit, port, bcmPortControlAbility, &nAbility);
    if (rv) {
      return rv;
    }

    rv = _bcm_bm9600_xb_iport_enable_set(unit, port, nAbility, enable);
    if (rv) {
      return rv;
    }

    if ((bFirstEnable) && (SOC_SBX_CFG_BM9600(unit)->uDeviceMode== SOC_SBX_BME_LCM_MODE)) {

	SAND_HAL_RMW_FIELD(unit, PL, XB_CONFIG0, SOFT_RESET, 1);
#if 000
	LOG_CLI((BSL_META_U(unit,
                            "xb_config out of reset for port(%d)\n"), port));
#endif
	thin_delay(250000);
	SAND_HAL_RMW_FIELD(unit, PL, XB_CONFIG0, SOFT_RESET, 0);
    }
    bFirstEnable = FALSE;

    rv = soc_phyctrl_enable_set(unit, port, enable);

    SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_CONFIG0, SOFT_RESET, ~enable);
    SAND_HAL_RMW_FIELD_STRIDE(unit, PL, SI, port, SI_CONFIG0, ENABLE, enable);


    if (IS_SCI_PORT(unit, port)) {

#ifndef _BM9600_INA_ENABLE_ON_
	/* Enable in the INA when the SCI port is enabled */
	ina_blk = port;

	/* Update INA instance */
	ina_config = SAND_HAL_READ_STRIDE(unit, PL, INA, ina_blk, INA_CONFIG);
	if (enable) {
	    ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 1);
	}
	else {
	    ina_config = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, ina_config, 0);
	}
	SAND_HAL_WRITE_STRIDE(unit, PL, INA, ina_blk, INA_CONFIG, ina_config);
	
	if (enable) {
	    rv = soc_bm9600_ina_sysport_sync(unit, ina_blk);
	}
#endif /* _BM9600_INA_ENABLE_ON_ */
    }


    return rv;
}

int
bcm_bm9600_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    BM9600_PORT_CHECK(unit, port);
    return soc_phyctrl_enable_get(unit, port, enable);
}

int
bcm_bm9600_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask)
{
    int rv = BCM_E_NONE;

    
    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {

    } else {
        rv = BCM_E_PORT;
    }

    return rv;

}
