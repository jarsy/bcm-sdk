/*
 * $Id: port.c,v 1.184 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Port API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>

#include <soc/xaui.h>
#include <soc/phyctrl.h>
#include <soc/phyreg.h>
#include <soc/portmode.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>
#include <soc/mem.h>
#include <soc/mcm/memregs.h>
#include <soc/higig.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/link.h>
#include <bcm/types.h>
#include <bcm/stack.h>

#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/common/link.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/sirius.h>

#define SIRIUS_PORT(unit, port)        SOC_SBX_STATE(unit)->port_state->port_info[port]

#define SIRIUS_INIT_DONE(unit)         (SOC_SBX_STATE(unit)->port_state->init == TRUE)

#define SIRIUS_PORT_CHECK(unit, port)           \
  do {                                          \
      if (!SOC_PORT_VALID(unit, port)) {	\
	  return BCM_E_PARAM;                   \
      }                                         \
      if (! (IS_SFI_PORT(unit, port) ||		\
             IS_HG_PORT(unit, port)  ||         \
             IS_XE_PORT(unit, port)  ||         \
             IS_GE_PORT(unit, port)  ||         \
             IS_CPU_PORT(unit, port) ||         \
             IS_REQ_PORT(unit, port) ||         \
             IS_SCI_PORT(unit, port)))  {       \
            return BCM_E_PARAM;                 \
      }                                         \
  } while(0)

#define SIRIUS_PORT_IS_HG_LINE_PORT(unit, port)                                             \
             ( IS_HG_PORT(unit, port) &&                                                    \
               ((port - SOC_PORT_MIN(unit, hg)) < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) )

static int bcm_sirius_port_congestion_init(int unit);
static int bcm_sirius_port_congestion_deinit(int unit);

bcm_sbx_port_congestion_info_t *congestion_info[SOC_MAX_NUM_DEVICES];

extern int bcm_sbx_port_clear(int unit);

/*
 * Define:
 *      PORT_INIT
 * Purpose:
 *      Causes a routine to return BCM_E_INIT if port is not yet initialized.
 */
#undef PORT_INIT
#define PORT_INIT(unit) \
        if (SOC_SBX_STATE(unit)->port_state == NULL) { return BCM_E_INIT; }

#undef PORT_LOCK
#define PORT_LOCK(unit)  \
    if (sal_mutex_take(SOC_SBX_STATE(unit)->port_state->port_lock, sal_mutex_FOREVER)) { \
        /* Cound not obtain unit lock  */ \
        LOG_ERROR(BSL_LS_BCM_PORT, \
                  (BSL_META_U(unit, \
                              "unable to obtain PORT lock on unit %d\n"), \
                   unit)); \
        return BCM_E_INTERNAL; \
    }

#undef PORT_UNLOCK
#define PORT_UNLOCK(unit) \
    if (sal_mutex_give(SOC_SBX_STATE(unit)->port_state->port_lock)) { \
        /* Cound not obtain unit lock  */ \
        LOG_ERROR(BSL_LS_BCM_PORT, \
                  (BSL_META_U(unit, \
                              "unable to release PORT lock on unit %d\n"), \
                   unit)); \
        return BCM_E_INTERNAL; \
    }

#define SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(op)   \
  do {                                          \
      int __rv__;                               \
      if ((__rv__ = (op)) < 0) {                \
          PORT_UNLOCK(unit);                    \
          return(__rv__);                       \
      }                                         \
  } while(0)


static int
_bcm_sirius_port_to_serdes(int unit, bcm_port_t port, int *nSi);

int
bcm_sirius_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int rv = BCM_E_UNAVAIL;
    int    nSi = -1;
    uint32 uData;
    int sfi_port;

    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);

    if (SOC_IS_DETACHING(unit)) {
        return BCM_E_NONE;
    }

    PORT_LOCK(unit);

    enable = (enable) ? TRUE : FALSE;

    if (IS_GX_PORT(unit, port)) {
        /* enable higig port */
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        rv = BCM_E_NONE;
        if (!SAL_BOOT_BCMSIM)
            /* BCMSIM model doesn't support 64bits registers access for now,
             * and MAC_INIT has some 64bits registers access. Disable it
             * for now
             */
#endif
        {
	    if (enable) {
		rv = MAC_ENABLE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, TRUE);

		if (SOC_SUCCESS(rv)) {
		    rv = soc_phyctrl_enable_set(unit, port, TRUE);
		}
	    } else {
		rv = soc_phyctrl_enable_set(unit, port, FALSE);
            
		if (SOC_SUCCESS(rv)) {
		    rv = MAC_ENABLE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, FALSE);
		}
	    }
        }
    } else if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {

        if (IS_SFI_PORT(unit, port)) {
            /* Module id must be set before ports are enabled bcm_stk_modid_set() */
            if (SIRIUS_INIT_DONE(unit)) { 
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sbx_port_to_sfi_base_get(unit, SOC_SBX_CONTROL(unit)->node_id, port, &sfi_port));
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_update_lchan_for_port_enable_set(unit, sfi_port, enable));
            }
        }

        /* enable SFI/SCI port */
        /* map port to serdes */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(_bcm_sirius_port_to_serdes(unit, port, &nSi));

        rv = soc_phyctrl_enable_set(unit, port, enable);

        if (nSi < 12) {
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uData));
            soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uData, ENABLEf, (enable)?1:0);
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uData));		    
        } else {
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uData));
            soc_reg_field_set(unit, SI_CONFIG0r, &uData, ENABLEf, (enable)?1:0);
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uData));
        }

    } else if (IS_CPU_PORT(unit, port) || IS_REQ_PORT(unit, port)) {
        /* enable CPU port */
	rv = BCM_E_NONE;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_sirius_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int rv = BCM_E_UNAVAIL;

    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);
    PORT_LOCK(unit);

    if (IS_GX_PORT(unit, port)) {
        /* higig port */

        /* The PHY enable holds the port enable state set by the user.
         *      The MAC enable transitions up and down automatically via linkscan
         *      even if user port enable is always up.
         */
        rv = soc_phyctrl_enable_get(unit, port, enable);
    } else if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        /* SCI/SFI port */
        /* map port to serdes */
	rv = soc_phyctrl_enable_get(unit, port, enable);
    } else if (IS_CPU_PORT(unit, port)) {
        /* CPU port */
	*enable = 1;
	rv = BCM_E_NONE;
    } else if (IS_REQ_PORT(unit, port)) {
	*enable = 1;
	rv = BCM_E_NONE;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_enable_get: u=%d p=%d rv=%d enable=%d\n"),
              unit, port, rv, *enable));
    PORT_UNLOCK(unit);
    return rv;
}

/* calling function performs port lock */
STATIC int
_bcm_sirius_port_prbs_rx_setup(int unit, bcm_port_t port, int nSi, int bEnablePrbs)
{
   int port_speed;
   uint32 uRegValue;

    SOC_IF_ERROR_RETURN(bcm_sirius_port_speed_get(unit, port, &port_speed));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin.\n"), FUNCTION_NAME()));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: u=%d p=%d nSi=0x%x bEnablePrbs=0x%x\n"),
              FUNCTION_NAME(), unit, port, nSi, bEnablePrbs));
    if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "#### %s: PRBS Mode: si\n"), FUNCTION_NAME()));
	if (nSi < 12) {
	    SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
	    soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, ENABLEf, bEnablePrbs? 0:1);
	    SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
	    
	} else {
	    SOC_IF_ERROR_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
	    soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, ENABLEf, bEnablePrbs?0:1);
	    SOC_IF_ERROR_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
	}
	
	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed == 3125)) { 
	    
	    if (nSi < 12) {
		SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
		soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, RX_BYTE_SWAPf, bEnablePrbs? 0:1);
		SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
	    } else {
		SOC_IF_ERROR_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
		soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, RX_BYTE_SWAPf, bEnablePrbs? 0:1);
		SOC_IF_ERROR_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
	    }	    	    

	    if (port_speed == 6250) {	    
	    
                LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "#### %s: Doing scrambler phy stuff.\n"), FUNCTION_NAME()));
		SOC_IF_ERROR_RETURN
		    (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_SCRAMBLER,bEnablePrbs?0:1));
		
	    }
	}
	    
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "#### %s: Doing 8b10b phy stuff.\n"), FUNCTION_NAME()));
	SOC_IF_ERROR_RETURN
	  (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_8B10B,bEnablePrbs?0:1));


	if ((port_speed > 3125) && (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: Doing 64b66b phy stuff.\n"), FUNCTION_NAME()));
	    /* Disable or enable 64/66 encoding */
	    SOC_IF_ERROR_RETURN
		(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_64B66B, bEnablePrbs?0:1));
	}

        /* Enable the PRBS Monitor */
        if (nSi < 12) {
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
            soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, PRBS_MONITOR_ENABLEf, (bEnablePrbs)?1:0);
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: (Prbs_Mon_En) Writing nSi: %d, SC_TOP_SI_CONFIG0: 0x%08x\n"),
                      FUNCTION_NAME(), nSi, uRegValue));
        } else {
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
            soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, PRBS_MONITOR_ENABLEf, (bEnablePrbs)?1:0);
        	SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: (Prbs_Mon_En) Writing nSi: %d, SI_CONFIG0: 0x%08x\n"),
                      FUNCTION_NAME(), nSi, uRegValue));
        }

    }
    /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
    /* But, on the receive side, set the status select so that the receive status is set up for PRBS */
    else {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "#### %s: PRBS Mode: hc\n"), FUNCTION_NAME()));
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, 
				     SOC_PHY_CONTROL_PRBS_RX_ENABLE, 
				     bEnablePrbs));
	/* GNATS 19203, when enabling/disabling PRBS if 3.125G, must disable swapping */
	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed==3125)) {
	    
	    if (nSi < 12) {
		SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
		soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, RX_BYTE_SWAPf, (bEnablePrbs)?0:1);
		SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
		
	    } else {
		SOC_IF_ERROR_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
		soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, RX_BYTE_SWAPf, (bEnablePrbs)?0:1);
		SOC_IF_ERROR_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
	    }
	    
	}
	/* If we are enabling PRBS, we need to disable 8b10b encoding and comma detect */
	/* in the hypercore.  Otherwise, we need to enable 8b10b encoding and comma    */
	/* detect in the hypercore                                                     */
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "#### %s: 1: Doing 8b10b phy stuff.\n"), FUNCTION_NAME()));
	/* Disable 8b10b/comma detect or re-enable it */
	SOC_IF_ERROR_RETURN
	  (soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_8B10B, (bEnablePrbs)?0:1));

	if ((port_speed > 3125) && (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {

            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: 1: Doing 64b66b phy stuff.\n"), FUNCTION_NAME()));
	    /* Disable or enable 64/66 encoding */
	    SOC_IF_ERROR_RETURN
		(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_64B66B, bEnablePrbs?0:1));
	}
    }
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin. Done...\n"), FUNCTION_NAME()));
    return BCM_E_NONE;
}

/* calling function performs port lock/unlock */
STATIC int
_bcm_sirius_port_prbs_tx_setup(int unit, bcm_port_t port, int nSi, int bEnablePrbs)
{
    uint32 uRegValue;
   int port_speed;

    SOC_IF_ERROR_RETURN(bcm_sirius_port_speed_get(unit, port, &port_speed));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin.\n"), FUNCTION_NAME()));
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: u=%d p=%d nSi=0x%x bEnablePrbs=0x%x\n"),
              FUNCTION_NAME(), unit, port, nSi, bEnablePrbs));
    /* SI PRBS */
    if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
	if (nSi < 12) {
	    SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: (Prbs_Gen_En) Reading nSi: %d, SC_TOP_SI_CONFIG0: 0x%08x\n"),
                      FUNCTION_NAME(), nSi, uRegValue));
	    soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, PRBS_GENERATOR_ENABLEf, (bEnablePrbs)?1:0);
	    SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: (Prbs_Gen_En) Writing nSi: %d, SC_TOP_SI_CONFIG0: 0x%08x\n"),
                      FUNCTION_NAME(), nSi, uRegValue));
	} else {
	    SOC_IF_ERROR_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: (Prbs_Gen_En) Reading nSi: %d, SI_CONFIG0: 0x%08x\n"),
                      FUNCTION_NAME(), nSi, uRegValue));
	    soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, PRBS_GENERATOR_ENABLEf, (bEnablePrbs)?1:0);
	    SOC_IF_ERROR_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
            LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "#### %s: (Prbs_Gen_En) Writing nSi: %d, SI_CONFIG0: 0x%08x\n"),
                      FUNCTION_NAME(), nSi, uRegValue));
	}

	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed == 3125)) { 
	    
	    if (nSi < 12) {
		SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
		soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, TX_BYTE_SWAPf, (bEnablePrbs)?0:1);
		SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
	    } else {
		SOC_IF_ERROR_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
		soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, TX_BYTE_SWAPf, (bEnablePrbs)?0:1);
		SOC_IF_ERROR_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
	    }
	    

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
    }
    /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
    else {
	SOC_IF_ERROR_RETURN
	    (soc_phyctrl_control_set(unit, port, 
				     SOC_PHY_CONTROL_PRBS_TX_ENABLE, 
				     bEnablePrbs));

	if ((SOC_SBX_CFG(unit)->bSerdesEncoding) || (port_speed==3125)) {
	
	    if (nSi < 12) {
		SOC_IF_ERROR_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
		soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, TX_BYTE_SWAPf, (bEnablePrbs)?0:1);
		SOC_IF_ERROR_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
	    } else {
		SOC_IF_ERROR_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
		soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, TX_BYTE_SWAPf, (bEnablePrbs)?0:1);
		SOC_IF_ERROR_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
	    }
	    
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
    }
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin. Done...\n"), FUNCTION_NAME()));
    return BCM_E_NONE;
}


STATIC int
_bcm_sirius_port_ability_local_get(int unit, bcm_port_t port,
                                   bcm_port_ability_t *ability_mask)
{
    int rv = BCM_E_UNAVAIL;
    soc_port_ability_t             mac_ability, phy_ability;

    if (IS_GX_PORT(unit, port)) {
        /* Sirius only use xport as Higig interface, software report only limited ability */
        SOC_IF_ERROR_RETURN(soc_phyctrl_ability_local_get(unit, port, &phy_ability));

#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        sal_memset(&mac_ability, 0, sizeof(bcm_port_ability_t));
        if (!SAL_BOOT_BCMSIM)
            /* BCMSIM model doesn't support 64bits registers access for now,
             * and MAC_INIT has some 64bits registers access. Disable it
             * for now
             */
#endif
        {
        SOC_IF_ERROR_RETURN(MAC_ABILITY_LOCAL_GET(SIRIUS_PORT(unit, port).p_mac, unit,
                                                  port, &mac_ability));
        }

        /* Combine MAC and PHY abilities */
        ability_mask->speed_half_duplex  = mac_ability.speed_half_duplex & phy_ability.speed_half_duplex;
        ability_mask->speed_full_duplex  = mac_ability.speed_full_duplex & phy_ability.speed_full_duplex;
        ability_mask->pause     = mac_ability.pause & phy_ability.pause;
        if (phy_ability.interface == 0) {
            ability_mask->interface = mac_ability.interface;
        } else {
            ability_mask->interface = phy_ability.interface;
        }
        ability_mask->medium    = phy_ability.medium;
        ability_mask->loopback  = mac_ability.loopback | phy_ability.loopback | BCM_PORT_ABILITY_LB_NONE;
        ability_mask->flags     = mac_ability.flags | phy_ability.flags;

        /* for sirius, the HG port is used as higig interface only,
         *   don't report the abilities apply to front panel ports
         */
        ability_mask->speed_half_duplex = SOC_PA_ABILITY_NONE;   /* don't support half-duplex */

	if (IS_HG_PORT(unit, port)) {
	    ability_mask->pause             = SOC_PA_ABILITY_NONE;   /* no pause support */
	    ability_mask->interface         = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
	    ability_mask->medium            = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
	    ability_mask->flags             = SOC_PA_ABILITY_NONE;   /* no auto-neg or combo */
	}

        rv = BCM_E_NONE;
    } else if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        /* SFI/SCI port ability */
        ability_mask->speed_half_duplex = SOC_PA_ABILITY_NONE;   /* don't support half-duplex */
        ability_mask->speed_full_duplex = SOC_PA_ABILITY_NONE;   /* don't support this ability */
        ability_mask->pause             = SOC_PA_ABILITY_NONE;   /* no pause support */
        ability_mask->interface         = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
        ability_mask->medium            = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
        ability_mask->flags             = SOC_PA_ABILITY_NONE;   /* no auto-neg or combo */
        ability_mask->loopback          = BCM_PORT_ABILITY_LB_PHY;
        rv = BCM_E_NONE;
    } else if (IS_CPU_PORT(unit, port)) {
        /* CPU port ability */
        ability_mask->speed_half_duplex = SOC_PA_ABILITY_NONE;   /* don't support half-duplex */
        ability_mask->speed_full_duplex = SOC_PA_ABILITY_NONE;   /* don't support this ability */
        ability_mask->pause             = SOC_PA_ABILITY_NONE;   /* no pause support */
        ability_mask->interface         = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
        ability_mask->medium            = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
        ability_mask->flags             = SOC_PA_ABILITY_NONE;   /* no auto-neg or combo */
        ability_mask->loopback          = SOC_PA_ABILITY_NONE;
        rv = BCM_E_NONE;
    } else if (IS_REQ_PORT(unit, port)) {
	/* Requeue port ability */
        ability_mask->speed_half_duplex = SOC_PA_ABILITY_NONE;   /* don't support half-duplex */
        ability_mask->speed_full_duplex = SOC_PA_ABILITY_NONE;   /* don't support this ability */
        ability_mask->pause             = SOC_PA_ABILITY_NONE;   /* no pause support */
        ability_mask->interface         = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
        ability_mask->medium            = SOC_PA_ABILITY_NONE;   /* don't apply for non-front panel interface */
        ability_mask->flags             = SOC_PA_ABILITY_NONE;   /* no auto-neg or combo */
        ability_mask->loopback          = SOC_PA_ABILITY_NONE;
        rv = BCM_E_NONE;
    }

    return rv;
}

int
bcm_sirius_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask)
{
    int                 rv;
    bcm_port_ability_t  port_ability;

    PORT_LOCK(unit);

    rv = _bcm_sirius_port_ability_local_get(unit, port, &port_ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&port_ability, ability_mask);
    }

    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_ability_get: u=%d p=%d abil=0x%x rv=%d\n"),
              unit, port, *ability_mask, rv));

    return rv;
}

int
bcm_sirius_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int rv = BCM_E_UNAVAIL;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int subport = 0;
    int fifo = 0, egroup_num = -1, num_fifos = 0;
    int lb_phy, force_phy, enable;
    int mac_lb;

    /* Higig interface fix speed at 25G
     * SCI/SFI speed is driven by port ability
     */

    PORT_INIT(unit);
    if (!(BCM_GPORT_IS_CHILD(port)        ||
	  BCM_GPORT_IS_EGRESS_CHILD(port) ||
	  BCM_GPORT_IS_EGRESS_GROUP(port))) {
	SIRIUS_PORT_CHECK(unit, port);
    }
    PORT_LOCK(unit);

    /*
     * Set the subport speed
     */

    if (BCM_GPORT_IS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_GROUP(port)) {

        subport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, port, &subport, &egroup_num, &num_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Could not retrieve egroup information from port 0x%x\n"),
                       FUNCTION_NAME(), port));
	    PORT_UNLOCK(unit);
	    return rv;
	}
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	if (sp_info == NULL) {
            /* coverity[dead_error_begin] */
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_port %d, unit %d\n"),
	               FUNCTION_NAME(), subport, unit));
	    PORT_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

        sp_info->egroup[egroup_num].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = speed;
        for (fifo=0; fifo < sp_info->egroup[egroup_num].num_fifos; fifo++) {
            sp_info->egroup[egroup_num].port_speed[fifo] = speed;
        }
        rv = soc_sirius_config_ff(unit, subport, egroup_num, FF_MEM_UPDATE);

    } else if (IS_GX_PORT(unit, port)) {
        /*
	 * If port is in MAC loopback mode, do not try setting the PHY
	 * speed.  This allows MAC loopback at 10/100 even if the PHY is
	 * 1000 only.  Loopback diagnostic tests should enable loopback
	 * before setting the speed, and vice versa when cleaning up.
	 */

        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	  (MAC_LOOPBACK_GET(SIRIUS_PORT(unit, port).p_mac, unit, port, &mac_lb));

	force_phy = !mac_lb;

	if (speed == 0) {
	    /* if speed is 0, set the port speed to max */
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	      (bcm_port_speed_max(unit, port, &speed));    
	}

	if (force_phy) {
	    bcm_port_ability_t      mac_ability, phy_ability;
	    bcm_port_ability_t      requested_ability;

	    /* Make sure MAC can handle the requested speed. */
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	      (MAC_ABILITY_LOCAL_GET(SIRIUS_PORT(unit, port).p_mac, unit, port, &mac_ability));
	    requested_ability.speed_full_duplex = SOC_PA_SPEED(speed);
	    requested_ability.speed_half_duplex = SOC_PA_SPEED(speed);
	    LOG_INFO(BSL_LS_BCM_PHY,
                     (BSL_META_U(unit,
                                 "_bcm_port_speed_set: u=%u p=%d MAC FD speed %08X MAC HD speed %08X \
                                 Requested FD Speed %08X Requested HD Speed %08X\n"),
                      unit,
                      port,
                      mac_ability.speed_full_duplex,
                      mac_ability.speed_half_duplex,       
                      requested_ability.speed_full_duplex,
                      requested_ability.speed_half_duplex));

	    if (((mac_ability.speed_full_duplex &		\
		  requested_ability.speed_full_duplex) == 0) &&
		((mac_ability.speed_half_duplex &		\
		  requested_ability.speed_half_duplex) == 0) ) {
	        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "u=%d p=%d MAC doesn't support %d Mbps speed.\n"),
                             unit, port, speed));
		PORT_UNLOCK(unit);
		return SOC_E_CONFIG;
	    }

	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	      (soc_phyctrl_ability_local_get(unit, port, &phy_ability));
	    
	    if (((phy_ability.speed_full_duplex &		\
		  requested_ability.speed_full_duplex) == 0) &&
		((phy_ability.speed_half_duplex &		\
		  requested_ability.speed_half_duplex) == 0) ) {
	      
	      LOG_VERBOSE(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "u=%d p=%d PHY doesn't support %d Mbps speed.\n"),
                           unit, port, speed));
	      PORT_UNLOCK(unit);
	      return SOC_E_CONFIG;
	    }
	    
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	      (soc_phyctrl_auto_negotiate_set(unit, port, FALSE));
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	      (soc_phyctrl_speed_set(unit, port, speed));
	}
	
	/* Prevent PHY register access while resetting BigMAC and Fusion core */
	if (IS_HG_PORT(unit, port)) {
	    soc_phyctrl_enable_get(unit, port, &enable);
	    soc_phyctrl_enable_set(unit, port, 0);
	    soc_phyctrl_loopback_get(unit, port, &lb_phy);
	}
	
	rv = MAC_SPEED_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, speed);
        if (SOC_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "MAC_SPEED_SET failed: %s\n"), bcm_errmsg(rv)));
        }
	
	/* Restore PHY register access */
	if (IS_HG_PORT(unit, port)) {
	    soc_phyctrl_enable_set(unit, port, enable);
	    soc_phyctrl_loopback_set(unit, port, lb_phy, TRUE);
	} 
   }

    PORT_UNLOCK(unit);
    return rv;
}

/*
 *  Function
 *    bcm_sirius_port_subport_getnext
 *  Purpose
 *    Iterate through all subports that belong to a port
 *  Arguments
 *    (in) int unit = the unit number
 *    (in) bcm_port_t port = the base higig port number
 *    (in) flags_out = exclude the subport with these flags set
 *    (in/out) bcm_gport_t *subport = ptr to current & where to put next
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    To get first subport of a port, 'subport' must be BCM_GPORT_INVALID on
 *    entry.  Other values will be interpreted as the current subport and the
 *    next one will be retrieved (but invalid values may be errors).
 *
 *    If getting past the final subport of a port, 'subport' will be set to
 *    BCM_GPORT_INVALID, and result code will still be BCM_E_NONE.
 */
int
bcm_sirius_port_subport_getnext(int unit,
                                bcm_port_t port,
				uint32 flags_out,
                                bcm_gport_t *subport)
{
    int result = BCM_E_NONE;
    bcm_port_t sport;
    bcm_module_t module;
    bcm_module_t myModId;
    unsigned int portIndex;
    int intfPort, portOffset, matched;
    bcm_sbx_subport_info_t *sp_info;

    if (subport == NULL) {
        /* not bothering, since this is in/out argument */
        return BCM_E_PARAM;
    }
    if (!(IS_GX_PORT(unit, port) || IS_REQ_PORT(unit, port))) {
        /* port is not a higig or requeue port */
        return BCM_E_PARAM;
    }
    result = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        /* can't get modid; give up */
        return result;
    }

    /* convert the subport into an internal ID */
    if (BCM_GPORT_IS_CHILD(*subport)) {
        sport = BCM_GPORT_CHILD_PORT_GET(*subport);
        module = BCM_GPORT_CHILD_MODID_GET(*subport);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(*subport)) {
        sport = BCM_GPORT_EGRESS_CHILD_PORT_GET(*subport);
        module = BCM_GPORT_EGRESS_CHILD_MODID_GET(*subport);
    } else if (BCM_GPORT_INVALID == *subport) {
        sport = -1;
        module = myModId;
    } else {
        return BCM_E_PARAM;
    }
    if (myModId != module) {
        /* not local GPORT but the higig is local */
        return BCM_E_NOT_FOUND;
    }

    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);
    PORT_LOCK(unit);
    if (IS_GX_PORT(unit, port) || IS_REQ_PORT(unit, port)) {
	/* searching through all possible fabric ports, if caller doesn't specify a child gport
	 * (fabric port), then return the first child gport on the interface port. Otherwise
	 * return the next child gport on the same interface port. Skipping ports on other
	 * interface ports or not valid
	 */
	matched = FALSE;
	result = BCM_E_NOT_FOUND;
	sp_info = SOC_SBX_STATE(unit)->port_state->subport_info;
	for (portIndex = 0; portIndex < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; portIndex++, sp_info++) {
	    if ( (sp_info->valid != TRUE) || (sp_info->flags & flags_out) ) {
		/* skip all invalid fabric ports, skip any subport with any flags in the flags_out */
		continue;		
	    }
	    result = bcm_sbx_port_get_intf_portoffset(unit, portIndex, &intfPort, &portOffset);
	    if (result != BCM_E_NONE) {
		/* skip all fabric port without these info */
		result = BCM_E_NOT_FOUND;
		continue;
	    }

	    if (IS_GX_PORT(unit, port)) {
		if (intfPort != (SB_FAB_DEVICE_SIRIUS_HG0_INTF + SOC_PORT_OFFSET(unit, port))) {
		    result = BCM_E_NOT_FOUND;
		    continue;
		}
	    } else {
		if (intfPort != (SB_FAB_DEVICE_SIRIUS_RQ0_INTF + SOC_PORT_OFFSET(unit, port))) {
		    result = BCM_E_NOT_FOUND;
		    continue;
		}
	    }

	    if (-1 == sport) {
		/* caller wants first subport this port */
		sport = portIndex;
		result = BCM_E_NONE;
		break;
	    } else {/* if (-1 == sport) */
		/* caller specified a particular subport, so wants next one */
		if (matched == TRUE) {
		    /* matched the specified subport, and found the next subport on the same port */
		    sport = portIndex;
		    result = BCM_E_NONE;
		    break;
		}
		
		if (portIndex == sport) {
		    /* matched the specified subport */
		    matched = TRUE;
		}
	    }
	    result = BCM_E_NOT_FOUND;
	}

        if (BCM_E_NONE == result) {
            if (BCM_GPORT_IS_CHILD(*subport)) {
                /* return a child port */
                BCM_GPORT_CHILD_SET(*subport, module, sport);
            } else {
                /* return an egress child port */
                BCM_GPORT_EGRESS_CHILD_SET(*subport, module, sport);
            }
        } else if (result == BCM_E_NOT_FOUND) {
	    /* don't report error, just return an invalid gport */
	    result = BCM_E_NONE;
	    *subport = BCM_GPORT_INVALID;
	}
    }
    PORT_UNLOCK(unit);
    return result;
}

int
bcm_sirius_port_subport_fifos_get(int unit, bcm_gport_t subport, uint32 flags, int *fifo_map, int *nbr_fifos, int max_size)
{
    int                      child_port;
    int                      rc = BCM_E_NONE;
    bcm_sbx_subport_info_t  *sp_info;
    int                      egrp_num, native_fifo_grps, cur_native_fifo, fifo_base, total_fifos;


    if (BCM_GPORT_IS_CHILD(subport)) {
        child_port = BCM_GPORT_CHILD_PORT_GET(subport);
    }
    else if BCM_GPORT_IS_EGRESS_CHILD(subport) {
        child_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(subport);
    }
    else {
        return(BCM_E_PARAM);
    }

    (*nbr_fifos) = 0;
    total_fifos = 0;

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[child_port]);

    if ( (sp_info->valid != TRUE) || ((sp_info->flags & flags) != flags) ) {
        return(rc); /* should never occur */
    }

    /* Process subport info */
    for (egrp_num = 0; egrp_num < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; egrp_num++) {
        if (sp_info->egroup[egrp_num].num_fifos == 0) {
            continue;
        }

        native_fifo_grps = sp_info->egroup[egrp_num].num_fifos / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE;
        native_fifo_grps = (sp_info->egroup[egrp_num].num_fifos % SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE) ? (native_fifo_grps + 1) : native_fifo_grps;

        for (cur_native_fifo = 0; cur_native_fifo < native_fifo_grps; cur_native_fifo++) {
            fifo_base = (sp_info->egroup[egrp_num].es_scheduler_level0_node +
                                (cur_native_fifo * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS));
            
            fifo_map[total_fifos] = fifo_base;
            total_fifos++;
            if (total_fifos == max_size) {
                break;
            }
        }
    }

    (*nbr_fifos) = total_fifos;

    return(rc);
}

/*
 *  Function
 *    bcm_sirius_port_all_subports_speed_get
 *  Purpose
 *    Get the combined speed of all subports on a higig
 *  Arguments
 *    (in) int unit = the unit on which to operate
 *    (in) bcm_port_t port = the higig port number
 *    (out) int *speed = the total speed
 *  Results
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Despite being signed numbers, all speeds must be non-negative.  The sum
 *    of all subport speeds must not exceed what can be represented by a signed
 *    integer.
 */
int
bcm_sirius_port_all_subports_speed_get(int unit, bcm_port_t port, int *speed)
{

    bcm_sbx_subport_info_t *sp_info = NULL;
    uint16 subport = 0;
    int rv = BCM_E_UNAVAIL;
    int idx = 0;
 
    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);
    PORT_LOCK(unit);

    if (IS_HG_PORT(unit, port)) {
        *speed = 0;
	for (subport=0; subport < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; subport++) {
	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ((sp_info == NULL) || (sp_info->valid == FALSE)) continue;
	    if (BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport) == port) {
		for (idx = 0; idx < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; idx++) {
                    if (0 == (sp_info->flags & (SBX_SUBPORT_FLAG_TRUNK_MCAST |
                                                SBX_SUBPORT_FLAG_TRUNK_UCAST))) {
                        /* only count if not used for internal work */
                        *speed += sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE];
                    }
		}
 	    }
	}
	rv = BCM_E_NONE;
    }
    PORT_UNLOCK(unit);

    return rv;
}


int
bcm_sirius_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int rv = BCM_E_UNAVAIL;
    int nSi = -1;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int subport = 0;
    int egroup_num = -1, num_fifos = 0;
    int mac_lb;
    uint32 uRegValue;

    PORT_INIT(unit);
    if (!(BCM_GPORT_IS_CHILD(port)        ||
	  BCM_GPORT_IS_EGRESS_CHILD(port) ||
	  BCM_GPORT_IS_EGRESS_GROUP(port))) {
	SIRIUS_PORT_CHECK(unit, port);
    }
    PORT_LOCK(unit);

    if (BCM_GPORT_IS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_GROUP(port)) {
        subport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, port, &subport, &egroup_num, &num_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Could not retrieve egroup information from port 0x%x\n"),
	               FUNCTION_NAME(), port));
	    PORT_UNLOCK(unit);
	    return rv;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	if (sp_info == NULL) {
            /* coverity[dead_error_begin] */
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_port %d, unit %d\n"),
	               FUNCTION_NAME(), subport, unit));
	    PORT_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

        *speed = sp_info->egroup[egroup_num].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE];
        rv = BCM_E_NONE;
    } else if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        /* SCI/SFI port speed driven by port ability */
        /* map port to serdes */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(_bcm_sirius_port_to_serdes(unit, port, &nSi));

        rv = BCM_E_NONE;
        switch (SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi]) {
            case BCM_PORT_ABILITY_DUAL_SFI:
            case BCM_PORT_ABILITY_SFI_SCI:
            case BCM_PORT_ABILITY_DUAL_SFI_LOCAL:
            /* case BCM_PORT_ABILITY_DUAL_SFI_LOOPBACK: */
                *speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
                break;
            case BCM_PORT_ABILITY_SFI_LOOPBACK:
                *speed = SOC_SBX_CFG(unit)->uSerdesSpeed/2;
                break; 
            case BCM_PORT_ABILITY_SFI:
		*speed = 3125;
		break;
            case BCM_PORT_ABILITY_SCI:
		if (nSi < 12) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG3r(unit, nSi, &uRegValue));
                    uRegValue = (soc_reg_field_get(unit, SC_TOP_SI_CONFIG3r, uRegValue, CH_MODEf));
		} else {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG3r(unit, nSi, &uRegValue));
                    uRegValue = (soc_reg_field_get(unit, SI_CONFIG3r, uRegValue, CH_MODEf));
		}
		if (uRegValue) { /* single channel */
		    *speed = 3125;
		} else {
		    *speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
		}
                break;
            default:
                rv = BCM_E_PARAM;
        }
    } else if (IS_GX_PORT(unit, port)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        if (!SAL_BOOT_BCMSIM)
	    /* BCMSIM model doesn't support 64bits registers access for now,
	     * and MAC_INIT has some 64bits registers access. Disable it
	     * for now
	     */
#endif
	{
        rv = MAC_LOOPBACK_GET(SIRIUS_PORT(unit, port).p_mac, unit, port, &mac_lb);
	if (BCM_SUCCESS(rv)) {
	    if (mac_lb) {
	        rv = MAC_SPEED_GET(SIRIUS_PORT(unit, port).p_mac, unit, port, speed);
	    } else {
	        rv = soc_phyctrl_speed_get(unit, port, speed);
		if (BCM_E_UNAVAIL == rv) {
		    /* PHY driver doesn't support speed_get. Get the speed from
		     * MAC.
		     */
		    rv = MAC_SPEED_GET(SIRIUS_PORT(unit, port).p_mac, unit, port, speed);
		}
		if (IS_HG_PORT(unit, port) && *speed < 10000) {
		  *speed = 0;
		}
	    }
        }
	}
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        else {
            /* a speed is needed even if it's fake */
            if (IS_HG_PORT(unit, port)) {
                *speed = 25000;
            } else if (IS_XE_PORT(unit, port)) {
                *speed = 10000;
            } else if (IS_GE_PORT(unit, port)) {
                *speed = 1000;
            } else {
                *speed = 100;
            }
            rv = BCM_E_NONE;
        }
#endif
    } else {
        PORT_UNLOCK(unit);
        return BCM_E_PORT;
    }

    PORT_UNLOCK(unit);

    return rv;
}


int
bcm_sirius_port_link_get(int unit, bcm_port_t port, int *up)
{
    int rv = BCM_E_UNAVAIL;
    int    nSi = -1;
    int    timeAligned         = FALSE;
    int    byteAligned         = FALSE;
    uint32 uSiState, uSiConfig3, uSfiPortConfig0;
    soc_persist_t *sop = SOC_PERSIST(unit);

    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);
    PORT_LOCK(unit);

    *up = FALSE;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin.\n"), FUNCTION_NAME()));
    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        /* make sure byte aligned, and even/odd channels are both aligned to report up */

        /* map port to serdes */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(_bcm_sirius_port_to_serdes(unit, port, &nSi));
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "#### %s: u=%d p=%d mapped to Si=%d\n"),
                  FUNCTION_NAME(), unit, port, nSi));

        if (nSi < 12) {

	    /* SDK-30949 */
	    /* If the state indicates that we are not byte aligned regardless of sticky state,  */
	    /* report link as down.  This is because the sticky state might be 0 if the link is */
	    /* down and has been down continuously after being checked and cleared.             */
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_STATEr(unit, nSi, &uSiState));
	    if ((soc_reg_field_get(unit, SC_TOP_SI_STATEr, uSiState, MSM_RUN_BYTE_ALIGNMENTf) == 1)) {
	        PORT_UNLOCK(unit);
		*up = FALSE;
		return(BCM_E_NONE);
	    }

            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_STICKY_STATEr(unit, nSi, &uSiState));
            if ( soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_MSM_LOST_BYTE_ALIGNMENTf) != 1) { 
                byteAligned = TRUE;

		if (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_MSM_RUN_BYTE_ALIGNMENTf) == 1) {
		    byteAligned = FALSE;
		}

                if ((soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_EVEN_IDLEf) != 1) &&
                    (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_EVEN_SOT_SEARCHf) != 1) &&
		    (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_EVEN_SOT_EARLYf) != 1) &&
		    (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_EVEN_SOT_MISSINGf) != 1) ) {
                    /* Even channel is aligned -- do we need to check the odd channel? */
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG3r(unit, nSi, &uSiConfig3));
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SFI_PORT_CONFIG0r(unit, (nSi<2)?(nSi+20):((nSi-2)*2+1), &uSfiPortConfig0));
		    
		    if ( (soc_reg_field_get(unit, SC_TOP_SI_CONFIG3r, uSiConfig3, CH_MODEf) == 0) &&
			 ((SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi]) != BCM_PORT_ABILITY_DUAL_SFI_LOCAL) &&
			 (soc_reg_field_get(unit, SC_TOP_SFI_PORT_CONFIG0r, uSfiPortConfig0, LOOPBACK_ENf) == 0) ) {
		
			if ((soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_ODD_IDLEf) != 1) &&
			    (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_ODD_SOT_SEARCHf) != 1) &&
			    (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_ODD_SOT_EARLYf) != 1) &&
			    (soc_reg_field_get(unit, SC_TOP_SI_STICKY_STATEr, uSiState, S_TASM_ODD_SOT_MISSINGf) != 1) ) {
			    timeAligned = TRUE;
			} else {
			    timeAligned = FALSE;
			}
                    } else {
			timeAligned = TRUE;
		    }

		    if (IS_SCI_PORT(unit, port) && !(IS_SFI_PORT(unit, port))) {
			/* sci only port should ignore odd channel */
			timeAligned = TRUE;
		    }
                }
            }
	    /* Write to clear */
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_STICKY_STATEr(unit, nSi, uSiState));

        } else {

	    /* SDK-30949 */
	    /* If the state indicates that we are not byte aligned regardless of sticky state,  */
	    /* report link as down.  This is because the sticky state might be 0 if the link is */
	    /* down and has been down continuously after being checked and cleared.             */
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_STATEr(unit, (nSi - 12), &uSiState));
	    if ((soc_reg_field_get(unit, SI_STATEr, uSiState, MSM_RUN_BYTE_ALIGNMENTf) == 1)) {
	        PORT_UNLOCK(unit);
		*up = FALSE;
		return(BCM_E_NONE);
	    }

            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_STICKY_STATEr(unit, (nSi - 12), &uSiState));

            if (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_MSM_LOST_BYTE_ALIGNMENTf) == 0) {
                byteAligned = TRUE;

		if (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_MSM_RUN_BYTE_ALIGNMENTf) == 1) {
		    byteAligned = FALSE;
		}

                if ((soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_EVEN_IDLEf) != 1) &&
                    (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_EVEN_SOT_SEARCHf) != 1) &&
		    (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_EVEN_SOT_EARLYf) != 1) &&
		    (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_EVEN_SOT_MISSINGf) != 1) ) {
		    
                    /* Even channel is aligned -- do we need to check the odd channel? */
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG3r(unit, (nSi - 12), &uSiConfig3));
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SFI_PORT_CONFIG0r(unit, (nSi - 12)*2+1, &uSfiPortConfig0));

		    if ( (soc_reg_field_get(unit, SI_CONFIG3r, uSiConfig3, CH_MODEf) == 0) &&
			 ((SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi]) != BCM_PORT_ABILITY_DUAL_SFI_LOCAL) &&
			 (soc_reg_field_get(unit, SFI_PORT_CONFIG0r, uSfiPortConfig0, LOOPBACK_ENf) == 0) ) {
			
			if ((soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_ODD_IDLEf) != 1) &&
			    (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_ODD_SOT_SEARCHf) != 1) &&
			    (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_ODD_SOT_EARLYf) != 1) &&
			    (soc_reg_field_get(unit, SI_STICKY_STATEr, uSiState, S_TASM_ODD_SOT_MISSINGf) != 1) ) {
			    timeAligned = TRUE;
			} else {
			    timeAligned = FALSE;
			}
                    } else {
			timeAligned = TRUE;
		    }
                }
            }
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_STICKY_STATEr(unit, (nSi - 12), uSiState));
        }
        if (timeAligned && byteAligned) {
            *up = TRUE;
	    SOC_PBMP_PORT_ADD(sop->lc_pbm_link, port);
        } else {
	    SOC_PBMP_PORT_REMOVE(sop->lc_pbm_link, port);
	    LOG_INFO(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "port %d siState 0x%x\n"), port, uSiState));
	}	

	rv = BCM_E_NONE;
    } else if (IS_GX_PORT(unit, port)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        if (!SAL_BOOT_BCMSIM)
	    /* BCMSIM model doesn't support 64bits registers access for now,
	     * and MAC_INIT has some 64bits registers access. Disable it
	     * for now
	     */
#endif
	{

        rv = _bcm_link_get(unit, port, up);

        if (rv == BCM_E_DISABLED) {
            rv =  soc_phyctrl_link_get(unit, port, up);
        }

	}

        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "bcm_port_linkstatus_get: u=%d p=%d up=%d rv=%d\n"),
                  unit, port, *up, rv));
    } else if (IS_CPU_PORT(unit, port)) {
        *up = TRUE;
	rv = BCM_E_NONE;
    } else if (IS_REQ_PORT(unit, port)) {
	*up = TRUE;
	rv = BCM_E_NONE;
    } else {
        rv = BCM_E_PORT;
    }

    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin. Done...\n"), FUNCTION_NAME()));
    return rv;
}
int
bcm_sirius_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    soc_persist_t *sop = SOC_PERSIST(unit);

    SIRIUS_PORT_CHECK(unit, port);

    /* If linkscan maintains the state, return the saved state */
    if ((bcm_linkscan_enable_port_get(unit, port) == BCM_E_DISABLED) ||
        (IS_GX_PORT(unit, port))) {

	return bcm_sirius_port_link_get(unit, port, up);

    } else {
	*up = (SOC_PBMP_MEMBER(sop->lc_pbm_link, port)) ?
	    BCM_PORT_LINK_STATUS_UP : BCM_PORT_LINK_STATUS_DOWN;

    }
    return(BCM_E_NONE);
}

int
bcm_sirius_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    int rv = BCM_E_NONE;
    phy_ctrl_t   *int_pc;
    phy_driver_t *pd;
    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        PORT_LOCK(unit);

	INT_PHY_INIT_CHECK(unit, port);
	int_pc = INT_PHY_SW_STATE(unit, port);
	pd = int_pc->pd;
	rv = PHY_LOOPBACK_SET(pd, unit, port, loopback != BCM_PORT_LOOPBACK_NONE);
	PORT_UNLOCK(unit);
	return rv;
    } else if (IS_GX_PORT(unit, port)) {
        /* for Higig, loopback on MAC or PHY
         * support BCM_PORT_LOOPBACK_NONE
         *         BCM_PORT_LOOPBACK_MAC
         *         BCM_PORT_LOOPBACK_PHY
         */
        if (!(loopback == BCM_PORT_LOOPBACK_NONE)) {
             rv = _bcm_link_force(unit, port, TRUE, FALSE);
        }

        PORT_LOCK(unit);

        if (BCM_SUCCESS(rv)) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        rv = BCM_E_NONE;
        if (!SAL_BOOT_BCMSIM)
            /* BCMSIM model doesn't support 64bits registers access for now,
             * and MAC_INIT has some 64bits registers access. Disable it
             * for now
             */
#endif
        {
            rv = MAC_LOOPBACK_SET(SIRIUS_PORT(unit, port).p_mac, unit, port,
                                  (loopback == BCM_PORT_LOOPBACK_MAC));
        }
        }
        if (BCM_SUCCESS(rv)) {
            rv = soc_phyctrl_loopback_set(unit, port,
                                     (loopback == BCM_PORT_LOOPBACK_PHY), TRUE);
        }

        PORT_UNLOCK(unit);                  /* unlock before link call */

        if ((loopback == BCM_PORT_LOOPBACK_NONE) || !BCM_SUCCESS(rv)) {
            _bcm_link_force(unit, port, FALSE, DONT_CARE);
        } else {
            /* Enable only MAC instead of calling bcm_port_enable_set so
             * that this API doesn't silently enable the port if the
             * port is disabled by application.
             */
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
            rv = BCM_E_NONE;
            if (!SAL_BOOT_BCMSIM)
                /* BCMSIM model doesn't support 64bits registers access for now,
                 * and MAC_INIT has some 64bits registers access. Disable it
                 * for now
                 */
#endif
            {
                rv = MAC_ENABLE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, TRUE);
            }

            if (BCM_SUCCESS(rv)) {
                /* Make sure that the link status is updated only after the
                 * MAC is enabled so that link_mask2 is set before the
                 * calling thread synchronizes with linkscan thread in
                 * _bcm_link_force call.
                 * If the link is forced before MAC is enabled, there could
                 * be a race condition in _soc_link_update where linkscan
                 * may use an old view of link_mask2 and override the
                 * EPC_LINK_BMAP after the mac_enable_set updates
                 * link_mask2 and EPC_LINK_BMAP.
                 */
                rv = _bcm_link_force(unit, port, TRUE, TRUE);
            }
        }
    } else if (IS_CPU_PORT(unit, port)) {
        /* no CPU loopback */
        rv = BCM_E_UNAVAIL;
    } else if (IS_REQ_PORT(unit, port)) {
	/* no Requeue loopback */
	rv = BCM_E_UNAVAIL;
    } else {
        rv = BCM_E_PORT;
    }

    return rv;
}

int
bcm_sirius_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{
    int rv = BCM_E_NONE;
    int    phy_lb = 0;
    int    mac_lb = 0;
    phy_ctrl_t   *int_pc;
    phy_driver_t *pd;

    PORT_INIT(unit);
    SIRIUS_PORT_CHECK(unit, port);
    PORT_LOCK(unit);

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {


	INT_PHY_INIT_CHECK(unit, port);
	int_pc = INT_PHY_SW_STATE(unit, port);
	pd = int_pc->pd;
	rv = PHY_LOOPBACK_GET(pd, unit, port, loopback);

    } else if (IS_GX_PORT(unit, port)) {
        /* for Higig, loopback on MAC or PHY
         * support BCM_PORT_LOOPBACK_NONE
         *         BCM_PORT_LOOPBACK_MAC
         *         BCM_PORT_LOOPBACK_PHY
         */
        rv = soc_phyctrl_loopback_get(unit, port, &phy_lb);
        if (rv >= 0) {
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        mac_lb = FALSE;
        if (!SAL_BOOT_BCMSIM)
            /* BCMSIM model doesn't support 64bits registers access for now,
             * and MAC_INIT has some 64bits registers access. Disable it
             * for now
             */
#endif
        {
            rv = MAC_LOOPBACK_GET(SIRIUS_PORT(unit, port).p_mac, unit, port, &mac_lb);
        }
        }

        if (rv >= 0) {
            if (mac_lb) {
                *loopback = BCM_PORT_LOOPBACK_MAC;
            } else if (phy_lb) {
                *loopback = BCM_PORT_LOOPBACK_PHY;
            } else {
                *loopback = BCM_PORT_LOOPBACK_NONE;
            }
        }
    } else if (IS_CPU_PORT(unit, port)) {
        /* no CPU loopback */
        *loopback = BCM_PORT_LOOPBACK_NONE;
        rv = BCM_E_NONE;
    } else if (IS_REQ_PORT(unit, port)) {
	/* no Requeue loopback */
	*loopback = BCM_PORT_LOOPBACK_NONE;
	rv = BCM_E_NONE;
    } else {
        rv = BCM_E_PORT;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "bcm_port_loopback_get: u=%d p=%d lb=%d rv=%d\n"),
              unit, port, *loopback, rv));

    PORT_UNLOCK(unit);

    return rv;
}

/*
 *  In hardware, the ingress interfaces (HGX 0..3, RQ 0..1, CPU) are numbered
 *  contiguously.  Unhappily, they are not so here.  This function must map
 *  HiGig 0..3 (ports 1..4) and RQ 0..1 (ports 5..6) and CPU (port 0).  Note
 *  that in certain modes, HiGig 0..3 are actually 10G 0..3, but even in these
 *  modes they have the default queue setting presented here.
 */
static int
_bcm_sirius_port_control_default_queue_access(int unit,
                                              int write,
                                              bcm_port_t port,
                                              unsigned int portIndex,
                                              int *value)
{
    int regData = -1;
    unsigned int hwPort;
    int rv;

    if (BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port) ||
        (!BCM_GPORT_IS_SET(port))) {
        if (SB_FAB_DEVICE_SIRIUS_CPU_PORT == portIndex) {
            /* CPU */
            hwPort = 0;
        } else if (0 == portIndex) {
            /* HiGig 0 */
            hwPort = 1;
        } else if (1 == portIndex) {
            /* HiGig 1 */
            hwPort = 2;
        } else if (2 == portIndex) {
            /* HiGig 2 */
            hwPort = 3;
        } else if (3 == portIndex) {
            /* HiGig 3 */
            hwPort = 4;
        } else if (SOC_PORT_MIN(unit, req) == portIndex) {
            /* requeue 0 */
            hwPort = 5;
        } else if (SOC_PORT_MIN(unit, req) + 1 == portIndex) {
            /* requeue 1 */
            hwPort = 6;
        } else {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unit %d gport %08X(%d) not valid for"
                                   " bcmPortControlDefaultQueue\n"),
                       unit,
                       port,
                       portIndex));
            return BCM_E_PORT;
        }
        if (write) {
            /* writing, so parse caller's value into queue ID */
            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(*value)) {
                regData = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(*value);
            } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(*value)) {
                regData = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(*value);
            } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(*value)) {
                regData = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(*value);
            } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(*value)) {
                regData = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(*value);
            } else if (BCM_GPORT_INVALID == *value) {
                regData = 0xFFFF; /* invalid -> queue 0xFFFF elsewhere */
            } else if (!BCM_GPORT_IS_SET(*value)) {
                /* write raw queue ID even though read gets a gport */
                regData = *value;
            } else {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_U(unit,
                                      "bcmPortControlDefaultQueue can not accept"
                                       " value %08X\n"), *value));
                return BCM_E_PARAM;
            }
        }
        rv = soc_sirius_port_control_default_queue_access(unit,
                                                          write,
                                                          hwPort,
                                                          &regData);
        if (!write) {
            /* reading, so parse result into caller provided space */
            if (SOC_E_NONE == rv) {
                rv = _bcm_sirius_fabric_qsel_entry_gport_extrapolate(unit,
                                                                     regData,
                                                                     value);
            } /* if (SOC_E_NONE == rv) */
        } /* if (!write) */
        if (SOC_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "unable to %s bcmPortControlDefaultQueue"
                                   " for unit %d port %08X (value %08X): %d (%s)\n"),
                       write?"write":"read",
                       unit,
                       port,
                       *value,
                       rv,
                       _SHR_ERRMSG(rv)));
        }
    } else { /* if (port is MODPORT or EGRESS_MODPORT or not(GPORT)) */
        /* this control is not on CHILD ports */
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "port control bcmPortControlDefaultQueue is only"
                               " valid for modport or egress_modport\n")));
        return BCM_E_UNAVAIL;
    } /* if (port is MODPORT or EGRESS_MODPORT or not(GPORT)) */
    return rv;
}




int
bcm_sirius_port_control_set(int unit, bcm_port_t port,
                            bcm_port_control_t type, int value)
{
    int rv = BCM_E_UNAVAIL;
    int    nSi = -1;
    uint32 uPhyAddr;
    uint32 uLaneAddr;
    uint32 uRegValue, uData;
    uint32 uChMode;
    unsigned int hgSi;
    unsigned int nP = ~0;
    int sfi_port = 0;
    int lxbar = -1, lxbar1;
    uint32 sci_mux_enable0, sci_mux_enable1, sci_mux_enable2, sci_mux_enable3;
    bcm_module_t myMod,pMod;
    eg_fdm_port_regs_entry_t regs_entry;
    soc_info_t *si;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin.\n"), FUNCTION_NAME()));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: u=%d p=%d type=%d value=0x%x\n"),
              FUNCTION_NAME(), unit, port, type, value));

    if (type == bcmPortControlPFCTransmit) {
	/* per-cos enable/disable PFC transmit */
	BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG0r(unit, &uRegValue));
	soc_reg_field_set(unit, QM_PFC_CONFIG0r, &uRegValue, PFC_COS_ENABLEf, (value & 0xFF));
	BCM_IF_ERROR_RETURN(WRITE_QM_PFC_CONFIG0r(unit, uRegValue));
	
	SOC_SBX_CFG(unit)->pfc_cos_enable = (value & 0xFF);
	return BCM_E_NONE;
    }

    if (BCM_GPORT_IS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_CHILD(port) ||
        BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        rv = bcm_stk_my_modid_get(unit, &myMod);
        if (BCM_E_NONE != rv) {
            /* that call should not fail */
            return rv;
        }
        /*
         *  port control includes bcmPortControlFabircKnockoutId, which is a
         *  setting that appears to apply to 'child' ports and possibly other
         *  gport types, which have their own requirements...
         */
        if (BCM_GPORT_IS_CHILD(port)) {
            nSi = BCM_GPORT_CHILD_PORT_GET(port);
            pMod = BCM_GPORT_CHILD_MODID_GET(port);
        } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
            nSi = BCM_GPORT_EGRESS_CHILD_PORT_GET(port);
            pMod = BCM_GPORT_EGRESS_CHILD_MODID_GET(port);
        } else if (BCM_GPORT_IS_MODPORT(port)) {
            pMod = BCM_GPORT_MODPORT_MODID_GET(port);
            nP = BCM_GPORT_MODPORT_PORT_GET(port);
            rv = bcm_sirius_aggregate_gport_translate(unit,
                                                      0 /* flags */,
                                                      myMod,
                                                      pMod,
                                                      port,
                                                      &hgSi,
                                                      NULL,
                                                      NULL);
            if (BCM_E_NONE == rv) {
                nSi = hgSi;
            }
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(port)) {
            pMod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
            nP = BCM_GPORT_EGRESS_MODPORT_PORT_GET(port);
            rv = bcm_sirius_aggregate_gport_translate(unit,
                                                      0 /* flags */,
                                                      myMod,
                                                      pMod,
                                                      port,
                                                      &hgSi,
                                                      NULL,
                                                      NULL);
            if (BCM_E_NONE == rv) {
                nSi = hgSi;
            }
        } else {
            /* should never get here */
            return BCM_E_PARAM;
        }
        if (myMod != pMod) {
            /* not my module */
            return BCM_E_NONE;
        }
        rv = BCM_E_UNAVAIL;
    } else {
        /*
         *  ...but if it's not one of those types, it must be a local port
         *  number and has to use the normal local port check.
         */
        SIRIUS_PORT_CHECK(unit, port);
    }

    si = &SOC_INFO(unit);

    PORT_LOCK(unit);

    if (BCM_GPORT_IS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_CHILD(port) ||
        BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* CHILD or EGRESS_CHILD or MODPORT or EGRESS_MODPORT */
        switch (type) {
	    case bcmPortControlFabricKnockoutId:
		if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= nSi) &&
		    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= nSi)) {
		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, nSi, &regs_entry));
		    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
			soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0xFFFF));
			soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFF);
		    } else {
			soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0x3FFF) << 2);
			soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFC);
		    }
		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_write(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ALL, nSi, &regs_entry));
		    rv = BCM_E_NONE;
		} else {
		    rv = BCM_E_PORT;
		}
		break;
            case bcmPortControlDefaultQueue:
                rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                                   TRUE,
                                                                   port,
                                                                   nP,
                                                                   &value);
                break;
	    default:
                rv = BCM_E_PORT;
                port = nP;

                /*
                 *  ...but if it's not one of those types, it must be a local port
                 *  number and has to use the normal local port check.
                 */
                SIRIUS_PORT_CHECK(unit, port);
        }
    } else
 
    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        /* map port to serdes */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(_bcm_sirius_port_to_serdes(unit, port, &nSi));

        /* map serdes to phy address */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_sirius_serdes_phyaddr(unit, nSi, &uPhyAddr, &uLaneAddr));

        switch (type) {
            case bcmPortControlPrbsMode:
                if (value == 0) {
                    /* PRBS in phy - lower layer */
                    SOC_SBX_STATE(unit)->port_state->uPrbsModeSi = 0;
                }
                else {
                    /* PRBS in SI block */
                    SOC_SBX_STATE(unit)->port_state->uPrbsModeSi = 1;
                }
                break;
            case bcmPortControlPrbsPolynomial:
                /* PRBS in SI */
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if ((value != BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1) &&
                        (value != BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1)) {
                        PORT_UNLOCK(unit);
                        return BCM_E_PARAM;
                    }

                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
                        soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, PRBS_POLY_SELECTf, value);
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
                        soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, PRBS_POLY_SELECTf, value);
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
                    }
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
		    PORT_UNLOCK(unit);
                    return BCM_E_PARAM;
                    break;
		}


		  SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
		      (soc_phyctrl_control_set(unit, port, 
					       SOC_PHY_CONTROL_PRBS_POLYNOMIAL, 
					       value));
		  		}
		break;

            case bcmPortControlPrbsTxInvertData:
                /* SI PRBS */
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
                        soc_reg_field_set(unit, SC_TOP_SI_CONFIG0r, &uRegValue, PRBS_INVERTf, (value)?1:0);
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_CONFIG0r(unit, nSi, uRegValue));
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
                        soc_reg_field_set(unit, SI_CONFIG0r, &uRegValue, PRBS_INVERTf, (value)?1:0);
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_CONFIG0r(unit, (nSi - 12), uRegValue));
                    }
                }
                /* Hypercore PRBS */
                else {

		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
			(soc_phyctrl_control_set(unit, port, 
						 SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA, 
						 value));
		}
		break;
            case bcmPortControlPrbsForceTxError:
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    /* Read the SI_PRBS_STATUS register will clear the error count.
                       We retrieve the state from the stored software state. This is
                       Assuming all other fields are read only
                    */
                    uRegValue = 0;
                    if (nSi < 12) {
                        soc_reg_field_set(unit, SC_TOP_SI_PRBS_STATUSr, &uRegValue, FORCE_ERRORf, (value)?1:0);
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_PRBS_STATUSr(unit, nSi, uRegValue));
                    } else {
                        soc_reg_field_set(unit, SI_PRBS_STATUSr, &uRegValue, FORCE_ERRORf, (value)?1:0);
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_PRBS_STATUSr(unit, (nSi - 12), uRegValue));
                    }
                    /* store the force tx state */
                    SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[nSi] = (value)?1:0;
                }
                else {
                    /* Not supported in Hypercore */
                    PORT_UNLOCK(unit);
                    return BCM_E_UNAVAIL;
                }
                break;
            case bcmPortControlPrbsTxEnable:
		rv = _bcm_sirius_port_prbs_tx_setup(unit, port, nSi, value);
        if(rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_PORT,
              (BSL_META_U(unit,
                          "ERROR: _bcm_sirius_port_prbs_tx_setup\n")));
        }
		break;
	    case bcmPortControlPrbsRxEnable:
		rv = _bcm_sirius_port_prbs_rx_setup(unit, port, nSi, value);
        if(rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_PORT,
              (BSL_META_U(unit,
                          "ERROR: _bcm_sirius_port_prbs_rx_setup\n")));
        }
		break;
	    case bcmPortControlSerdesDriverStrength:
		if ( (value >LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX) || (value < 0)) {
		    PORT_UNLOCK(unit);
		    return BCM_E_PARAM;
		}
		SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[nSi].uDriverStrength = value;
		soc_sirius_config_linkdriver(unit, nSi, &(SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[nSi]));
		break;
	    case bcmPortControlSerdesDriverEqualization:
		if ( (value > LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX) || (value < 0)) {
		    PORT_UNLOCK(unit);
                    return BCM_E_PARAM;
                }
                SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[nSi].uDriverEqualization = value;
                soc_sirius_config_linkdriver(unit, nSi, &(SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[nSi]));
                break;
            case bcmPortControlSerdesDriverEqualizationFarEnd:
                if ( (value > LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX) || (value < 0)) {
                    PORT_UNLOCK(unit);
                    return BCM_E_PARAM;
                }
                SOC_SBX_STATE(unit)->port_state->uDriverEqualizationFarEnd[nSi] = value;
                break;
            case bcmPortControlSerdesDriverTune:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_set(unit, port, 
                                             SOC_PHY_CONTROL_SERDES_DRIVER_TUNE,
                                             value));
                break;        
            case bcmPortControlAbility:

                /* If the port was SFI and may have a new ability, unmap it */
                if ((IS_SFI_PORT(unit, port)) && (value == BCM_PORT_ABILITY_SCI)) {
                    sfi_port = port - SOC_PORT_MIN(unit, sfi);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_unmap_link_status(unit, sfi_port));
                }

                /* Read to get current value of config3 */
                if (value == BCM_PORT_ABILITY_SFI) {
                    /* Single channel SFI */
                    uChMode = 1;
                    
                    SBX_ADD_PORT(sfi, port);
                    SBX_REMOVE_PORT(sci, port);
                    sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
                                 "sfi%d", port);
                } else if (value == BCM_PORT_ABILITY_DUAL_SFI) {
                    
                    if (SOC_SBX_CFG(unit)->uSerdesSpeed > 3125) {
                        /* Dual channel on sirius if speed greater than 3125 */
                        uChMode = 0;
                    } else {
                        /* Single channel SCI */
                        uChMode = 1;
                    }
                    
                    SBX_ADD_PORT(sfi, port);
                    SBX_REMOVE_PORT(sci, port);

                    sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
                                 "sfi%d", (port-SOC_PORT_MIN(unit,sfi)));
                } else if (value == BCM_PORT_ABILITY_SCI) {
                    if (SOC_SBX_CFG(unit)->uSerdesSpeed > 3125) {
                        /* Dual channel on sirius if speed greater than 3125 */
                        uChMode = 0;
		    } else {
                        /* Single channel SCI */
                        uChMode = 1;
                    }
                    
                    SBX_ADD_PORT(sci, port);
                    SBX_REMOVE_PORT(sfi, port);
                    sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
                                 "sci%d", (port-SOC_PORT_MIN(unit,sfi)-SB_FAB_DEVICE_SIRIUS_SFI_LINKS));
                } else if (value == BCM_PORT_ABILITY_SFI_SCI) {
                    /* Dual channel SFI/SCI */
                    uChMode = 0;
                    
                    SBX_ADD_PORT(sci, port);
                    SBX_ADD_PORT(sfi, port);
                    sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
                                 "sci_sfi%d", (port-SOC_PORT_MIN(unit,sfi)-SB_FAB_DEVICE_SIRIUS_SFI_LINKS));
                    
                }
                else if (value == BCM_PORT_ABILITY_DUAL_SFI_LOCAL) {
                    /* Dual channel SFI */
                    
                    if (SOC_SBX_CFG(unit)->uSerdesSpeed > 3125) {
		      /* Dual channel on sirius if speed greater than 3125 */
                        uChMode = 0;
                    } else {
                        /* Single channel SCI */
                        uChMode = 1;
                    }
                    
                    SBX_ADD_PORT(sfi, port);
                    SBX_REMOVE_PORT(sci, port);
                    
                    sal_snprintf(SOC_PORT_NAME(unit,port), sizeof(si->port_name[port]),
                                 "sfi%d", (port-SOC_PORT_MIN(unit,sfi)));
                } else if (value == BCM_PORT_ABILITY_SFI_LOOPBACK) {
                    /* Dual channel SFI; one channel in local loopback */
                    uChMode = 0;
                    
                    SBX_ADD_PORT(sfi, port);
                    SBX_REMOVE_PORT(sci, port);
                    
                } else {
                    PORT_UNLOCK(unit);
                    return BCM_E_PARAM;
                }
                
                if (nSi < 12) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG3r(unit, nSi, &uData));
                    soc_reg_field_set(unit, SC_TOP_SI_CONFIG3r, &uData, CH_MODEf, uChMode);
                    soc_reg_field_set(unit, SC_TOP_SI_CONFIG3r, &uData, EVEN_CH_DATA_SELECTf, 1);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_CONFIG3r(unit, nSi, uData));
                } else {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG3r(unit, (nSi - 12), &uData));
                    soc_reg_field_set(unit, SI_CONFIG3r, &uData, CH_MODEf, uChMode);
                    soc_reg_field_set(unit, SI_CONFIG3r, &uData, EVEN_CH_DATA_SELECTf, 1);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_CONFIG3r(unit, (nSi - 12), uData));
                }

                if (IS_SFI_PORT(unit, port)) {

                    sfi_port = port - SOC_PORT_MIN(unit, sfi);

                    /* Perform any updates required to the fabric channels when changing the ability
                     */
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_update_lchan_for_port_ability(unit, sfi_port, value,
                                                                                                      SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi]));
 
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sbx_fabric_xbar_for_sfi_get(unit, SOC_SBX_CONTROL(unit)->node_id, sfi_port, &lxbar, &lxbar1));
                    if (BCM_SBX_CROSSBAR_VALID(lxbar)) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_map_link_status(unit, lxbar, sfi_port));
                    }
                    if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_map_link_status(unit, lxbar1, sfi_port));
                    }
                } else {
                    sfi_port = port - SOC_PORT_MIN(unit, sfi);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_unmap_link_status(unit, sfi_port));
                }


                rv = soc_sirius_ability_matching_speed_set(unit, nSi, value);
                if(rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: soc_sirius_ability_matching_speed_set\n")));
                }
                break;

	    case bcmPortControlArbiterActive:

		/* Assign port to arbiter 0 or arbiter 1 */
		if ((value == 0) || (value == 1)) {
		    
		    /*
		     *  nSi0 sci_sfi0( 31)  HC65/4/0  sci0
		     *  nSi2 sfi0    (  9)  HC65/4/2  sci0
		     *  nSi3 sfi1    ( 10)  HC65/4/3  sci0
		     *
		     *  nSi1 sci_sfi1( 32)  HC65/4/1  sci1
		     *  nSi4 sfi2    ( 11)  HC65/5/0  sci1
		     *  nSi5 sfi3    ( 12)  HC65/5/1  sci1
		     */
		    if (value == 0) { /* ARB 0 */
			
			if ((SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi] != BCM_PORT_ABILITY_SFI_SCI) &&
			    (SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi] != BCM_PORT_ABILITY_SCI)) {
			    LOG_ERROR(BSL_LS_BCM_PORT,
			              (BSL_META_U(unit,
			                          "ERROR: ability must be SCI for port(%d) if setting arbiter active\n"),
			               port));

			    rv = BCM_E_PARAM;
			    PORT_UNLOCK(unit);
			    return rv;
			}
			
			switch (nSi) {
			    case (0):
				/* 4/0 to sci 0 */
				sci_mux_enable0 = 0;
				sci_mux_enable1 = 0;
				break;
			    case (2):
				/* 4/2 to sci 0 */
				sci_mux_enable0 = 1;
				sci_mux_enable1 = 0;
				break;
			    case (3):
				/* 4/3 to sci 0 */
				sci_mux_enable0 = 0;
				sci_mux_enable1 = 1;
				break;
			    default:
				LOG_ERROR(BSL_LS_BCM_PORT,
				          (BSL_META_U(unit,
				                      "ERROR: unit(%d) Cannot set port(%d) si(%d) ability(%d) to active arbiter(0) invalid configuration\n"),
				           unit, port, nSi, SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi]));
				LOG_ERROR(BSL_LS_BCM_PORT,
				          (BSL_META_U(unit,
				                      "Valid configurations for arbiter id 0, can use ports sfi0, sfi1, sci0 if ability set to SCI or SFI_SCI\n")));
				PORT_UNLOCK(unit);
				return BCM_E_PARAM;
				break;
			}
			
			SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_SW_RESETr(unit, &uData));
			soc_reg_field_set(unit, SC_SW_RESETr, &uData, SCI_MUX_ENABLE_0f, sci_mux_enable0);
			soc_reg_field_set(unit, SC_SW_RESETr, &uData, SCI_MUX_ENABLE_1f, sci_mux_enable1);
			SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_SW_RESETr(unit, uData));
			
		    } else if (value == 1) { /* ARB 1 */
			
			if ((SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi] != BCM_PORT_ABILITY_SFI_SCI) &&
			    (SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi] != BCM_PORT_ABILITY_SCI)) {
			    LOG_ERROR(BSL_LS_BCM_PORT,
			              (BSL_META_U(unit,
			                          "ERROR: ability must be SCI for port(%d) if setting arbiter active\n"),
			               port));
			    PORT_UNLOCK(unit);
			    rv = BCM_E_PARAM;
			    return rv;
			}
			
			switch (nSi) {
			    case (1):
				/* 4/1 to sci 1 */
				sci_mux_enable2 = 0;
				sci_mux_enable3 = 0;
				break;
			    case (4):
				/* 5/0 to sci 1 */
				sci_mux_enable2 = 1;
				sci_mux_enable3 = 0;
				break;
			    case (5):
				/* 5/1 to sci 1 */
				sci_mux_enable2 = 0;
				sci_mux_enable3 = 1;
				break;
			    default:
				LOG_ERROR(BSL_LS_BCM_PORT,
				          (BSL_META_U(unit,
				                      "ERROR: unit(%d) Cannot set port(%d) si(%d) ability(%d) to active arbiter(1) invalid configuration\n"),
				           unit, port, nSi, SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi]));
				LOG_ERROR(BSL_LS_BCM_PORT,
				          (BSL_META_U(unit,
				                      "Valid configurations for arbiter id 0, can use ports sfi2, sfi3, sci1 if ability set to SCI or SFI_SCI\n")));
				PORT_UNLOCK(unit);
				return BCM_E_PARAM;
				break;
			}
			SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_SW_RESETr(unit, &uData));
			soc_reg_field_set(unit, SC_SW_RESETr, &uData, SCI_MUX_ENABLE_2f, sci_mux_enable2);
			soc_reg_field_set(unit, SC_SW_RESETr, &uData, SCI_MUX_ENABLE_3f, sci_mux_enable3);
			SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_SW_RESETr(unit, uData));
			
			
		    } else { /* DISABLE */
                        /* coverity[dead_error_begin] */
			LOG_ERROR(BSL_LS_BCM_PORT,
			          (BSL_META_U(unit,
			                      "Disable currently unsupported, not required to switch between SCI muxing\n")));
			PORT_UNLOCK(unit);
			return BCM_E_PARAM;
		    }
		}
		break;
	    case bcmPortControlRxEnable:
                if (nSi < 12) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_SD_CONFIGr(unit, nSi, &uData));
                    soc_reg_field_set(unit, SC_TOP_SI_SD_CONFIGr, &uData, RX_PWRDWNf, (value)?0:1);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_SD_CONFIGr(unit, nSi, uData));
                } else {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_SD_CONFIGr(unit, (nSi - 12), &uData));
                    soc_reg_field_set(unit, SI_SD_CONFIGr, &uData, RX_PWRDWNf, (value)?0:1);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_SD_CONFIGr(unit, (nSi - 12), uData));
                }
                break;
            case bcmPortControlTxEnable:
                if (nSi < 12) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_SD_CONFIGr(unit, nSi, &uData));
                    soc_reg_field_set(unit, SC_TOP_SI_SD_CONFIGr, &uData, TX_PWRDWNf, (value)?0:1);
                    soc_reg_field_set(unit, SC_TOP_SI_SD_CONFIGr, &uData, TX_FIFO_RESETf, (value)?0:1);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_SD_CONFIGr(unit, nSi, uData));
                } else {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_SD_CONFIGr(unit, (nSi - 12), &uData));
                    soc_reg_field_set(unit, SI_SD_CONFIGr, &uData, TX_PWRDWNf, (value)?0:1);
                    soc_reg_field_set(unit, SI_SD_CONFIGr, &uData, TX_FIFO_RESETf, (value)?0:1);
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_SD_CONFIGr(unit, (nSi - 12), uData));
                }
                break;
           
            case bcmPortControlSerdesTuneMarginMode:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_set(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE,
                                             value));
                break;
            case bcmPortControlSerdesTuneMarginValue:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_set(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE,
                                             value));
                break;
            case bcmPortControlSerdesTuneMarginMax:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_set(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MAX,
                                             value));
                break;
	    case bcmPortControlArbiterForceDown:

	        if (!IS_SFI_PORT(unit, port)) {
                    PORT_UNLOCK(unit);
                    return BCM_E_PARAM;
	         }
		 sfi_port = port - SOC_PORT_MIN(unit, sfi);
            
                 /* Force the link to be reported down to the Polaris */
		 if (value == TRUE) {
                     SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_unmap_link_status(unit, sfi_port));
		 }
		 /* Revert force link reporting of down to the Polaris */
		 else {
		     SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sbx_fabric_xbar_for_sfi_get(unit, SOC_SBX_CONTROL(unit)->node_id, sfi_port, &lxbar, &lxbar1));
                     SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(bcm_sirius_fabric_map_link_status(unit, lxbar, sfi_port));
                 }
                break;

            case bcmPortControlPacketFlowMode:
                /* for child gport only??? */
            default:
                PORT_UNLOCK(unit);
                return BCM_E_UNAVAIL;
        }
	rv = BCM_E_NONE;

    } else if (IS_GX_PORT(unit, port)) {
        /* HiGig ports */
        rv = BCM_E_NONE;
        switch (type) {
        case bcmPortControlPrbsMode:
            if (value != 0) {
                PORT_UNLOCK(unit);
                return BCM_E_UNAVAIL;
            }
            break;

        case bcmPortControlPrbsTxEnable:
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                (soc_phyctrl_control_set(unit, port,
                                         SOC_PHY_CONTROL_PRBS_TX_ENABLE,
                                         value));
            break;
        case bcmPortControlPrbsPolynomial:
            SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
	        (soc_phyctrl_control_set(unit, port, 
				  SOC_PHY_CONTROL_PRBS_POLYNOMIAL, 
				  value));
	    break;
        case bcmPortControlPrbsRxEnable:
            break;
	case bcmPortControlSerdesDriverTune:
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
		(soc_phyctrl_control_set(unit, port, 
					 SOC_PHY_CONTROL_SERDES_DRIVER_TUNE,
					 value));
	    break;  

	case bcmPortControlFabricHeaderFormat:
	    switch (value) {
		case BCM_PORT_CONTROL_FABRIC_HEADER_CUSTOM:
		case BCM_PORT_CONTROL_FABRIC_HEADER_88230:
		    SOC_SBX_STATE(unit)->port_state->fabric_header_format = value;
		    /* 
		     * set DF bit to default header location
		     */
		    rv = soc_sirius_hw_update_trt(unit);
		    if (rv != BCM_E_NONE) {
			PORT_UNLOCK(unit);
			return rv;
		    }
		    rv = soc_sirius_hw_update_crt(unit);
		    if (rv != BCM_E_NONE) {
			PORT_UNLOCK(unit);
			return rv;
		    }
		    break;
		case BCM_PORT_CONTROL_FABRIC_HEADER_83200:
		case BCM_PORT_CONTROL_FABRIC_HEADER_88020_83200_88230_INTEROP:
		    rv = BCM_E_UNAVAIL;
		    break;
		default:
		    rv = BCM_E_PARAM;
		    break;
	    }
	    break;

        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0xFFFF));
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFF);
                    } else {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0x3FFF) << 2);
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFC);
                    }
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_write(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ALL, hgSi, &regs_entry));
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               TRUE,
                                                               port,
                                                               port,
                                                               &value);
            break;
        default:
	    PORT_UNLOCK(unit);
            return BCM_E_UNAVAIL;

        }
    } else if (IS_XE_PORT(unit, port)) {
        /* 10G ports */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0xFFFF));
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFF);
                    } else {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0x3FFF) << 2);
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFC);
                    }
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_write(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ALL, hgSi, &regs_entry));
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               TRUE,
                                                               port,
                                                               port,
                                                               &value);
            break;
        default:
            PORT_UNLOCK(unit);
            return BCM_E_UNAVAIL;
        }
    } else if (IS_CPU_PORT(unit, port)) {
        /* CPU port */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0xFFFF));
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFF);
                    } else {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0x3FFF) << 2);
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFC);
                    }
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_write(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ALL, hgSi, &regs_entry));
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               TRUE,
                                                               port,
                                                               port,
                                                               &value);
            break;
        default:
            PORT_UNLOCK(unit);
            return BCM_E_UNAVAIL;
        }
    } else if (IS_REQ_PORT(unit, port)) {
        /* requeue port */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0xFFFF));
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFF);
                    } else {
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf, (value & 0x3FFF) << 2);
                        soc_mem_field32_set(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_MASKf, 0xFFFC);
                    }
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_write(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ALL, hgSi, &regs_entry));
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               TRUE,
                                                               port,
                                                               port,
                                                               &value);
            break;
        default:
            PORT_UNLOCK(unit);
            return BCM_E_UNAVAIL;
        }
    }

    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin. Done...\n"), FUNCTION_NAME()));
    return rv;
}

int
bcm_sirius_port_control_get(int unit, bcm_port_t port,
                            bcm_port_control_t type, int *value)
{
    int rv = BCM_E_UNAVAIL;
    int    nSi = -1;
    uint32 uPhyAddr;
    uint32 uLaneAddr;
    uint32 uRegValue;
    uint32 uData = 0;
    uint32 sci_mux_enable0, sci_mux_enable1, sci_mux_enable2, sci_mux_enable3;
    bcm_module_t myMod,pMod;
    eg_fdm_port_regs_entry_t regs_entry;
    unsigned int hgSi;
    unsigned int nP = ~0;

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin.\n"), FUNCTION_NAME()));

    if (type == bcmPortControlPFCTransmit) {
	/* per-cos enable/disable PFC transmit */	
	*value = SOC_SBX_CFG(unit)->pfc_cos_enable;
	return BCM_E_NONE;
    }

    if (BCM_GPORT_IS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_CHILD(port) ||
        BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* CHILD or EGRESS_CHILD or MODPORT or EGRESS_MODPORT */
        SOC_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &myMod));
        /*
         *  port control includes bcmPortControlFabircKnockoutId, which is a
         *  setting that appears to apply to 'child' ports and possibly other
         *  gport types, which have their own requirements...
         */
        if (BCM_GPORT_IS_CHILD(port)) {
            nSi = BCM_GPORT_CHILD_PORT_GET(port);
            pMod = BCM_GPORT_CHILD_MODID_GET(port);
        } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
            nSi = BCM_GPORT_EGRESS_CHILD_PORT_GET(port);
            pMod = BCM_GPORT_EGRESS_CHILD_MODID_GET(port);
        } else if (BCM_GPORT_IS_MODPORT(port)) {
            pMod = BCM_GPORT_MODPORT_MODID_GET(port);
            nP = BCM_GPORT_MODPORT_PORT_GET(port);
            rv = bcm_sirius_aggregate_gport_translate(unit,
                                                      0 /* flags */,
                                                      myMod,
                                                      pMod,
                                                      port,
                                                      &hgSi,
                                                      NULL,
                                                      NULL);
            if (BCM_E_NONE == rv) {
                nSi = hgSi;
            }
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(port)) {
            pMod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
            nP = BCM_GPORT_EGRESS_MODPORT_PORT_GET(port);
            rv = bcm_sirius_aggregate_gport_translate(unit,
                                                      0 /* flags */,
                                                      myMod,
                                                      pMod,
                                                      port,
                                                      &hgSi,
                                                      NULL,
                                                      NULL);
            if (BCM_E_NONE == rv) {
                nSi = hgSi;
            }
        } else {
            /* should never get here */
            return BCM_E_PARAM;
        }
        if (myMod != pMod) {
            /* not my module */
            return BCM_E_NONE;
        }
    } else {
        /*
         *  ...but if it's not one of those types, it must be a local port
         *  number and has to use the normal local port check.
         */
        SIRIUS_PORT_CHECK(unit, port);
    }

    PORT_LOCK(unit);

    if (BCM_GPORT_IS_CHILD(port) ||
        BCM_GPORT_IS_EGRESS_CHILD(port) ||
        BCM_GPORT_IS_MODPORT(port) ||
        BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        /* CHILD or EGRESS_CHILD or MODPORT or EGRESS_MODPORT */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= nSi) &&
                (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= nSi)) {
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, nSi, &regs_entry));
                *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf);
                if (SOC_SBX_IF_PROTOCOL_XGS != SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                    *value = ((*value) >> 2) & 0x3FFF;
                }
                rv = BCM_E_NONE;
            } else {
                rv = BCM_E_PORT;
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               FALSE,
                                                               port,
                                                               nP,
                                                               value);
            break;
        default:
            rv = BCM_E_PORT;
            port = nP;

            /*
             *  ...but if it's not one of those types, it must be a local port
             *  number and has to use the normal local port check.
             */
            SIRIUS_PORT_CHECK(unit, port);
        }
    } 

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        /* map port to serdes */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(_bcm_sirius_port_to_serdes(unit, port, &nSi));

        /* map serdes to phy address */
        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_sirius_serdes_phyaddr(unit, nSi, &uPhyAddr, &uLaneAddr));

        switch (type) {
            case bcmPortControlPrbsPolynomial:
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
                        *value = soc_reg_field_get(unit, SC_TOP_SI_CONFIG0r, uRegValue, PRBS_POLY_SELECTf);
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
                        *value = soc_reg_field_get(unit, SI_CONFIG0r, uRegValue, PRBS_POLY_SELECTf);
                    }
                }
                else {
		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
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
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
                        *value = soc_reg_field_get(unit, SC_TOP_SI_CONFIG0r, uRegValue, PRBS_INVERTf);
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
                        *value = soc_reg_field_get(unit, SI_CONFIG0r, uRegValue, PRBS_INVERTf);
                    }
                }
                /* Hypercore PRBS */
                else {

		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
			(soc_phyctrl_control_get(unit, port, 
						 SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA,
						 &uData));
		    *value = uData;
                }
                break;
            case bcmPortControlPrbsForceTxError:
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    /* Read the SI_PRBS_STATUS register will clear the error count.
                       We retrieve the state from the stored software state
		    */
                    *value = SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[nSi];
                }
                else {
                    /* unavailable in hypercore */
                    PORT_UNLOCK(unit);
                    return BCM_E_UNAVAIL;
                }
                break;
            case bcmPortControlPrbsTxEnable:
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
                        *value = soc_reg_field_get(unit, SC_TOP_SI_CONFIG0r, uRegValue, PRBS_GENERATOR_ENABLEf);
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
                        *value = soc_reg_field_get(unit, SI_CONFIG0r, uRegValue, PRBS_GENERATOR_ENABLEf);
                    }
                }
                /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
                else {
		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
			(soc_phyctrl_control_get(unit, port, 
						 SOC_PHY_CONTROL_PRBS_TX_ENABLE,
						 &uData));
		    *value = uData;
		}
                break;
            case bcmPortControlPrbsRxEnable:
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG0r(unit, nSi, &uRegValue));
                        *value = soc_reg_field_get(unit, SC_TOP_SI_CONFIG0r, uRegValue, PRBS_MONITOR_ENABLEf);
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG0r(unit, (nSi - 12), &uRegValue));
                        *value = soc_reg_field_get(unit, SI_CONFIG0r, uRegValue, PRBS_MONITOR_ENABLEf);
                    }
                }
                /* Hypercore PRBS - note that in the Hypercore there is only 1 enable for both TX/RX */
                /* But, on the receive side, set the status select so that the receive status is set up for PRBS */
                else {

		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
			(soc_phyctrl_control_get(unit, port, 
						 SOC_PHY_CONTROL_PRBS_RX_ENABLE,
						 &uData));
		    *value = uData;

                }
                break;
            case bcmPortControlPrbsRxStatus:
	    
                /* SI PRBS */
                if (SOC_SBX_STATE(unit)->port_state->uPrbsModeSi) {
                    if (nSi < 12) {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_PRBS_STATUSr(unit, nSi, &uRegValue));
                        LOG_INFO(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "#### %s: Reading nSi: %d, SC_TOP_SI_PRBS_STATUS: 0x%08x\n"),
                                  FUNCTION_NAME(), nSi, uRegValue));
                        *value = soc_reg_field_get(unit, SC_TOP_SI_PRBS_STATUSr, uRegValue, PRBS_ERR_CNTf);
                    } else {
                        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_PRBS_STATUSr(unit, (nSi - 12), &uRegValue));
                        LOG_INFO(BSL_LS_BCM_PORT,
                                 (BSL_META_U(unit,
                                             "#### %s: Reading nSi: %d, SI_PRBS_STATUS: 0x%08x\n"),
                                  FUNCTION_NAME(), nSi, uRegValue));
                        *value = soc_reg_field_get(unit, SI_PRBS_STATUSr, uRegValue, PRBS_ERR_CNTf);
                    }
                }
                /* Hypercore PRBS */
                else {

		    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
			(soc_phyctrl_control_get(unit, port, 
						 SOC_PHY_CONTROL_PRBS_RX_STATUS,
						 &uData));
		    *value = uData;
                }
                break;
            case bcmPortControlSerdesDriverStrength:
                *value = SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[nSi].uDriverStrength;
                break;
	    case bcmPortControlSerdesDriverEqualization:
		*value = SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[nSi].uDriverEqualization;
                break;
            case bcmPortControlSerdesDriverEqualizationTuneStatusFarEnd:

		if (SOC_SBX_STATE(unit)->port_state->uDriverEqualizationFarEnd[nSi] == -1) {
		    LOG_ERROR(BSL_LS_BCM_PORT,
		              (BSL_META_U(unit,
		                          "User has not set the bcmPortControlSerdesDriverEqualizationFarEnd value, please set the near end and the far end value of this field\n")));
		    LOG_ERROR(BSL_LS_BCM_PORT,
		              (BSL_META_U(unit,
		                          "Please start with 0 and increase the value based upon the information provided through tuning\n")));
		    PORT_UNLOCK(unit);
		    return BCM_E_PARAM;
		}

                rv = (soc_phyctrl_control_get(unit, port, 
					      SOC_PHY_CONTROL_SERDES_DRIVER_EQUALIZATION_TUNE_STATUS_FAR_END,
					      &uData));

		if (rv) {
		    LOG_ERROR(BSL_LS_BCM_PORT,
		              (BSL_META_U(unit,
		                          "Error in tuning unit(%d) port(%d)\n"),
		               unit, port)); 
		} else {
		    
		    if (uData == FALSE) {
			
			if (SOC_SBX_STATE(unit)->port_state->uDriverEqualizationFarEnd[nSi] < 15) {
			    
			    LOG_CLI((BSL_META_U(unit,
                                                "unit(%d) port(%d): Please increase far end postcursor value from (%d) to (%d) and retune\n"), 
                                     unit,
                                     port,
                                     SOC_SBX_STATE(unit)->port_state->uDriverEqualizationFarEnd[nSi], 
                                     SOC_SBX_STATE(unit)->port_state->uDriverEqualizationFarEnd[nSi] + 1));
			    *value = uData;
			} 
		    }
		    else {
			LOG_CLI((BSL_META_U(unit,
                                            "unit(%d) port(%d): Far end postcursor value selection is good value(%d)\n"), 
                                 unit,
                                 port,
                                 SOC_SBX_STATE(unit)->port_state->uDriverEqualizationFarEnd[nSi]));

			*value = uData;
			rv = BCM_E_NONE;
		    }
		}
                break;
            case bcmPortControlAbility:
                *value = SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[nSi];
                break;

	    case bcmPortControlArbiterActive:

		/*
		 *  nSi0 sci_sfi0( 31)  HC65/4/0  sci0
		 *  nSi2 sfi0    (  9)  HC65/4/2  sci0
		 *  nSi3 sfi1    ( 10)  HC65/4/3  sci0
		 *
		 *  nSi1 sci_sfi1( 32)  HC65/4/1  sci1
		 *  nSi4 sfi2    ( 11)  HC65/5/0  sci1
		 *  nSi5 sfi3    ( 12)  HC65/5/1  sci1
		 */

		if (nSi > 5) {
		    LOG_ERROR(BSL_LS_BCM_PORT,
		              (BSL_META_U(unit,
		                          "unit(%d) ERROR: Port(%d) cannote be configured as active arbiter\n"),
		               unit, port));
		}

		SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_SW_RESETr(unit, &uData));
		sci_mux_enable0 = soc_reg_field_get(unit, SC_SW_RESETr, uData, SCI_MUX_ENABLE_0f);
		sci_mux_enable1 = soc_reg_field_get(unit, SC_SW_RESETr, uData, SCI_MUX_ENABLE_1f);

		SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_SW_RESETr(unit, &uData));
		sci_mux_enable2 = soc_reg_field_get(unit, SC_SW_RESETr, uData, SCI_MUX_ENABLE_2f);
		sci_mux_enable3 = soc_reg_field_get(unit, SC_SW_RESETr, uData, SCI_MUX_ENABLE_3f);			
		/* 4/0 to sci 0 */
		if ((sci_mux_enable0 == 0) && (sci_mux_enable1==0) && (nSi==0)) {
		    *value = 0;
		}
		/* 4/2 to sci 0 */
		else if ((sci_mux_enable0 == 1) && (sci_mux_enable1 == 0) && (nSi==2)) {
		    *value = 0;
		}
		/* 4/3 to sci 0 */
		else if ((sci_mux_enable0 == 0) && (sci_mux_enable1 == 1) && (nSi==3)) {
		    *value = 0;
		}
		/* 4/1 to sci 1 */
		else if ((sci_mux_enable2==0) && (sci_mux_enable3 == 0) && (nSi==1)) {
		    *value = 1;
		}
		/* 5/0 to sci 1 */
		else if ((sci_mux_enable2==1) && (sci_mux_enable3 == 0) && (nSi==4)) {
		    *value = 1;
		}
		/* 5/1 to sci 1 */
		else if ((sci_mux_enable2==0) && (sci_mux_enable3 == 1) && (nSi==5)) {
		    *value = 1;
		}
		else {
		    LOG_CLI((BSL_META_U(unit,
                                        "Unit(%d) Sirius device port(%d) not configured as sci link\n"), unit, port));
		    *value = -1;
		}
		break;

            case bcmPortControlRxEnable:
                if (nSi < 12) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_SD_CONFIGr(unit, nSi, &uRegValue));
                    if (soc_reg_field_get(unit, SC_TOP_SI_SD_CONFIGr, uRegValue, RX_PWRDWNf)) {
                        *value = 0;
                    } else {
                        *value = 1;
                    }
                } else {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_SD_CONFIGr(unit, (nSi - 12), &uRegValue));
                    if (soc_reg_field_get(unit, SI_SD_CONFIGr, uRegValue, RX_PWRDWNf)) {
                        *value = 0;
                    } else {
                        *value = 1;
                    }
                }
                break;
            case bcmPortControlTxEnable:
                if (nSi < 12) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_SD_CONFIGr(unit, nSi, &uRegValue));
                    if (soc_reg_field_get(unit, SC_TOP_SI_SD_CONFIGr, uRegValue, TX_PWRDWNf)) {
                        *value = 0;
                    } else {
                        *value = 1;
                    }
                } else {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_SD_CONFIGr(unit, (nSi - 12), &uRegValue));
                    if (soc_reg_field_get(unit, SI_SD_CONFIGr, uRegValue, TX_PWRDWNf)) {
                        *value = 0;
                    } else {
                        *value = 1;
                    }
                }
                break;
            case bcmPortControlSerdesTuneMarginMode:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_get(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE,
                                             &uData));
		    *value = uData;
                break;
            case bcmPortControlSerdesTuneMarginValue:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_get(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE,
                                             &uData));
		    *value = uData;
                break;
            case bcmPortControlSerdesTuneMarginMax:
                SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
                    (soc_phyctrl_control_get(unit, port,
                                             SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MAX,
                                             &uData));
		    *value = uData;
                break;
            default:
                PORT_UNLOCK(unit);
                return BCM_E_UNAVAIL;
	}

	rv = BCM_E_NONE;
    } else if (IS_HG_PORT(unit, port)) {
        /* HiGig port */
        rv = BCM_E_NONE;
        switch (type) {
        case bcmPortControlPrbsRxStatus:
          SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN
              (soc_phyctrl_control_get(unit, port,
                                       SOC_PHY_CONTROL_PRBS_RX_STATUS,
                                       &uData));
          *value = uData;
          break;

	case bcmPortControlFabricHeaderFormat:
	  *value = SOC_SBX_STATE(unit)->port_state->fabric_header_format;
	  break;

        case bcmPortControlSerdesDriverEqualizationTuneStatusFarEnd:
  
	  rv = (soc_phyctrl_control_get(unit, port, 
					SOC_PHY_CONTROL_SERDES_DRIVER_EQUALIZATION_TUNE_STATUS_FAR_END,
					&uData));

	  if (rv) {
	      LOG_ERROR(BSL_LS_BCM_PORT,
	                (BSL_META_U(unit,
	                            "Error in tuning unit(%d) port(%d)\n"),
	                 unit, port)); 
	  } else {
	      *value = uData;
	      if (uData == FALSE) {
		  LOG_ERROR(BSL_LS_BCM_PORT,
		            (BSL_META_U(unit,
		                        "Tuning failed unit(%d) port(%d)\n"),
		             unit, port)); 
		  rv = BCM_E_FAIL;
	      } else {
		  rv = BCM_E_NONE;
	      }	 
	  }
	
	  break;

        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf);
                    } else {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf) >> 2;
                    }
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               FALSE,
                                                               port,
                                                               port,
                                                               value);
            break;
        default:
            rv = BCM_E_PARAM;
        }
    } else if (IS_XE_PORT(unit, port)) {
        /* 10G port */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf);
                    } else {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf) >> 2;
                    }
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               FALSE,
                                                               port,
                                                               port,
                                                               value);
            break;
        default:
            rv = BCM_E_PARAM;
        }
    } else if (IS_CPU_PORT(unit, port)) {
        /* CPU port */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf);
                    } else {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf) >> 2;
                    }
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               FALSE,
                                                               port,
                                                               port,
                                                               value);
            break;
        default:
            rv = BCM_E_PARAM;
        }
    } else if (IS_REQ_PORT(unit, port)) {
        /* requeue port */
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            rv = bcm_stk_my_modid_get(unit, &myMod);
            if (BCM_E_NONE == rv) {
                rv = bcm_sirius_aggregate_gport_translate(unit,
                                                          0 /* flags */,
                                                          myMod,
                                                          myMod,
                                                          port,
                                                          &hgSi,
                                                          NULL,
                                                          NULL);
            }
            if (BCM_E_NONE == rv) {
                if ((SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_min <= hgSi) &&
                    (SOC_MEM_INFO(unit, EG_FDM_PORT_REGSm).index_max >= hgSi)) {
                    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(soc_mem_read(unit, EG_FDM_PORT_REGSm, MEM_BLOCK_ANY, hgSi, &regs_entry));
                    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf);
                    } else {
                        *value = soc_mem_field32_get(unit, EG_FDM_PORT_REGSm, &regs_entry, PORT_SIDf) >> 2;
                    }
                    rv = BCM_E_NONE;
                } else {
                    rv = BCM_E_PORT;
                }
            }
            break;
        case bcmPortControlDefaultQueue:
            rv = _bcm_sirius_port_control_default_queue_access(unit,
                                                               FALSE,
                                                               port,
                                                               port,
                                                               value);
            break;
        default:
            rv = BCM_E_PARAM;
        }
    }
    
    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: u=%d p=%d type=%d value=0x%x\n"),
              FUNCTION_NAME(), unit, port, type, *value));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "#### %s: Begin.\n"), FUNCTION_NAME()));
    return rv;
}

int
bcm_sirius_port_settings_init(int unit, bcm_port_t port)
{
    bcm_port_info_t info;

    bcm_port_info_t_init(&info);

    /* All the init speed, duplex, adv, autoneg don't
     * really apply to Higig port or SCI/SFI port
     */
#if 0
    if (IS_E_PORT(unit, port)) {
        val = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, -1);
	if (val != -1) {
	    info.speed = val;
	    info.action_mask |= BCM_PORT_ATTR_SPEED_MASK;
	}

	val = soc_property_port_get(unit, port, spn_PORT_INIT_DUPLEX, -1);
	if (val != -1) {
	    info.duplex = val;
	    info.action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
	}

	val = soc_property_port_get(unit, port, spn_PORT_INIT_ADV, -1);
	if (val != -1) {
	    info.local_advert = val;
	    info.action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
	}
	
	val = soc_property_port_get(unit, port, spn_PORT_INIT_AUTONEG, -1);
	if (val != -1) {
	    info.autoneg = val;
	    info.action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
	}
    }
#endif

    return bcm_port_selective_set(unit, port, &info);
}

STATIC int
_bcm_sirius_port_mode_setup(int unit, bcm_port_t port, int enable)
{
    soc_port_if_t    pif;

    /* Support GMII and MII for diag/bringup only */
    if (IS_GE_PORT(unit, port)) {
        pif = SOC_PORT_IF_GMII;
    } else if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) {
        pif = SOC_PORT_IF_XGMII;
    } else {
        pif = SOC_PORT_IF_MII;
    }

    /* PHY/MAC interface */
    if (INTERFACE_MAC(unit, port)) {
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_interface_set(unit, port, pif));
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
        if (!SAL_BOOT_BCMSIM)
            /* BCMSIM model doesn't support 64bits registers access for now,
             * and MAC_INIT has some 64bits registers access. Disable it
             * for now
             */
#endif
        {
            SOC_IF_ERROR_RETURN
                (MAC_INTERFACE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, pif));
            SOC_IF_ERROR_RETURN
               (MAC_ENABLE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, enable));
        }
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_sirius_port_probe(int unit, bcm_port_t p, int *okay)
{
    int                 rv;
    mac_driver_t        *macd;
    int                 nSi = 0;
    *okay = FALSE;

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_U(unit,
                            "Init port %d PHY...\n"), p));

    if ((rv = soc_phyctrl_probe(unit, p)) < 0) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d Port %s: Failed to probe PHY: %s\n"),
                  unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv)));
        return rv;
    }

    if (!SOC_WARM_BOOT(unit) && 
        ((rv = soc_phyctrl_init(unit, p)) < 0)) {

        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Unit %d Port %s: Failed to initialize PHY: %s\n"),
                  unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv)));
        return rv;
    }

    if (!IS_SFI_PORT(unit, p) && !IS_SCI_PORT(unit, p)) {

        /* Probe function should leave port disabled */
        if ((rv = soc_phyctrl_enable_set(unit, p, 0)) < 0) {
            return rv;
        }

	/*
	 * Currently initializing MAC after PHY is required because of
	 * phy_5690_notify_init().
	 */
	LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "Init port %d MAC...\n"), p));
	
	if ((rv = soc_mac_probe(unit, p, &macd)) < 0) {
	    LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Unit %d Port %s: Failed to probe MAC: %s\n"),
                      unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv)));
	    return rv;
	}
	
	SIRIUS_PORT(unit, p).p_mac = macd;
	
#ifndef SIRIUS_BCMSIM_64BITS_REG_SUPPORT
	if (!SAL_BOOT_BCMSIM)
	    /* BCMSIM model doesn't support 64bits registers access for now,
	     * and MAC_INIT has some 64bits registers access. Disable it
	     * for now
	     */
#endif
	{
	    if (!SOC_WARM_BOOT(unit) &&
		(rv = MAC_INIT(SIRIUS_PORT(unit, p).p_mac, unit, p)) < 0) {
		LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "Unit %d Port %s: Failed to initialize MAC: %s\n"),
                          unit, SOC_PORT_NAME(unit, p), soc_errmsg(rv)));
		return rv;
	    }
	    
	    /* Probe function should leave port disabled */
	    if ((rv = MAC_ENABLE_SET(SIRIUS_PORT(unit, p).p_mac, unit, p, 0)) < 0) {
		return rv;
	    }
	}
    }
    
    else {
	/* encoding == 0 => 64b66b */
	if ((SOC_SBX_CFG(unit)->uSerdesSpeed == 3125) &&
	    (SOC_SBX_CFG(unit)->bSerdesEncoding == 0)) {
	    LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Cannot set the speed to 3.125 with 64b66b encoding "
                                 "(bSerdesEncoding=0)\n")));
	    rv = SOC_E_PARAM;
	    return rv;
	}

	rv = soc_phyctrl_speed_set(unit, p, SOC_SBX_CFG(unit)->uSerdesSpeed);
        if (SOC_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "soc_phyctrl_speed_set failed port(%d): %s\n"), p, bcm_errmsg(rv)));
        }
	if (rv) {
	    return rv;
	}

	BCM_IF_ERROR_RETURN(_bcm_sirius_port_to_serdes(unit, p, &nSi));

	rv = (soc_sirius_hc_encoding_set(unit, nSi, SOC_SBX_CFG(unit)->bSerdesEncoding));
        if (SOC_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "soc_sirius_hc_encoding_set failed port(%d): %s\n"), p, bcm_errmsg(rv)));
        }
	if (rv) {
	    return rv;
	}
    }	

    *okay = TRUE;

    return BCM_E_NONE;
}

int
bcm_sirius_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int rv = BCM_E_NONE;
    bcm_port_t port;
    int okay;
    int chan_mode;
    uint32 regval = 0;
    int nSi;

    SOC_PBMP_CLEAR(*okay_pbmp);

    PORT_INIT(unit);
    PORT_LOCK(unit);

    PBMP_ITER(pbmp, port) {
        rv = _bcm_sirius_port_probe(unit, port, &okay);
        if (okay) {
            SOC_PBMP_PORT_ADD(*okay_pbmp, port);
        }
        if (rv < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Port probe failed on port %s\n"),
                      SOC_PORT_NAME(unit, port)));
            break;
        }

	if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
	    SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(_bcm_sirius_port_to_serdes(unit, port, &nSi));
	  	
	    /* chan_mode = ((pInitParams->sc.bEvenChannelOn[nSi] == TRUE) && (pInitParams->sc.bOddChannelOn[nSi] == TRUE)) ? 0 : 1; */
	    chan_mode = 0;
	
	    if (nSi < 12) {
	        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SC_TOP_SI_CONFIG3r(unit, nSi, &regval));
		soc_reg_field_set(unit, SC_TOP_SI_CONFIG3r, &regval, JIT_TOLERANCEf, 30); /* pInitParams->sc.uJitTolerance[nSi] */
		soc_reg_field_set(unit, SC_TOP_SI_CONFIG3r, &regval, CH_MODEf, chan_mode);
		SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SC_TOP_SI_CONFIG3r(unit, nSi, regval));
	    } else {
	        SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(READ_SI_CONFIG3r(unit, (nSi - 12), &regval));
		soc_reg_field_set(unit, SI_CONFIG3r, &regval, JIT_TOLERANCEf, 30); /* pInitParams->sc.uJitTolerance[nSi] */
		soc_reg_field_set(unit, SI_CONFIG3r, &regval, CH_MODEf, chan_mode);
		SIRIUS_SOC_IF_ERROR_UNLOCK_RETURN(WRITE_SI_CONFIG3r(unit, (nSi - 12), regval));
	    }
	}
    }

    PORT_UNLOCK(unit);

    return rv;
}

int
bcm_sirius_port_init(int unit)
{
    int                 rv = BCM_E_NONE, port_enable, eg=0, fifo=0;
    bcm_port_t          p;
    pbmp_t              okay_ports;
    pbmp_t              all_ports;
    char                pfmtok[SOC_PBMP_FMT_LEN],
                        pfmtall[SOC_PBMP_FMT_LEN];

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "bcm_port_init: unit %d\n"), unit));

    if (unit >= BCM_MAX_NUM_UNITS) {
        rv = BCM_E_PARAM;
        return (rv);
    }

    if (SOC_SBX_STATE(unit)->port_state->port_info == NULL) {
        SOC_SBX_STATE(unit)->port_state->port_info = sal_alloc(sizeof(bcm_sbx_port_info_t) * SOC_MAX_NUM_PORTS,
                                        "port_info");
        if (SOC_SBX_STATE(unit)->port_state->port_info == NULL) {
            rv = BCM_E_MEMORY;
            goto error;
        }
        sal_memset(SOC_SBX_STATE(unit)->port_state->port_info, 0,
                   sizeof(bcm_sbx_port_info_t) * SOC_MAX_NUM_PORTS);
    }

    if (SOC_SBX_STATE(unit)->port_state->subport_info == NULL) {
        SOC_SBX_STATE(unit)->port_state->subport_info = sal_alloc(sizeof(bcm_sbx_subport_info_t)
                                                                  * SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS,
                                                                  "subport_info");
        if (SOC_SBX_STATE(unit)->port_state->subport_info == NULL) {
            rv = BCM_E_MEMORY;
            goto error;
        }

        /* Only include members that need to be set other than 0 */
        sal_memset(SOC_SBX_STATE(unit)->port_state->subport_info, 0,
                   sizeof(bcm_sbx_subport_info_t) * SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS);
	for (p = 0; p < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; p++) {
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
		SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].ef_fcd = BCM_INT_SBX_INVALID_FCD;
		SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].nef_fcd = BCM_INT_SBX_INVALID_FCD;
		SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].es_scheduler_level0_node = -1;
		SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].egroup_gport = -1;
                for (fifo=0; fifo <  SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; fifo++) {
                    SOC_SBX_STATE(unit)->port_state->subport_info[p].egroup[eg].fcd[fifo] = -1;
                }
	    }
	}
    }
    SOC_SBX_STATE(unit)->port_state->fabric_header_format = BCM_PORT_CONTROL_FABRIC_HEADER_88230;

    /* alloc memory for port info, alloc semophore for port */
    if ((SOC_SBX_STATE(unit)->port_state->port_lock = sal_mutex_create("PORT")) == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "port init: unit %d unable to create port lock\n"),
                   unit));
        rv = BCM_E_MEMORY;
        goto error;
    }

    SOC_SBX_STATE(unit)->port_state->cpu_fabric_port = -1;

    if ((rv = soc_phy_common_init(unit)) != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Error unit %d:  Failed common PHY driver init: %s\n"),
                   unit, bcm_errmsg(rv)));
        goto error;
    }

    if ((rv = bcm_sirius_port_congestion_init(unit)) != BCM_E_NONE) {
        goto error;
    }

    /* Probe for ports */
    BCM_PBMP_ASSIGN(all_ports, PBMP_XE_ALL(unit));
    BCM_PBMP_OR(all_ports, PBMP_HG_ALL(unit));
    BCM_PBMP_OR(all_ports, PBMP_GE_ALL(unit));
    BCM_PBMP_OR(all_ports, PBMP_SFI_ALL(unit));
    BCM_PBMP_OR(all_ports, PBMP_SCI_ALL(unit));

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

    /* Probe and initialize MAC and PHY drivers for ports that were OK */
    PBMP_ITER(okay_ports, p) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "bcm_port_init: unit %d port %s\n"),
                     unit, SOC_PORT_NAME(unit, p)));

        if ((rv = _bcm_sirius_port_mode_setup(unit, p, TRUE)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to set initial mode: %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }

        if ((rv = bcm_sirius_port_settings_init(unit, p)) < 0) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Warning: Unit %d Port %s: "
                                 "Failed to configure initial settings: %s\n"),
                      unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv)));
        }


        if (soc_property_port_get(unit, p, "diag_rx_pause_enable", 0) > 0) {
            if (bcm_port_pause_set(unit, p, -1, 1) != SOC_E_NONE) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "%s: Unable to set RXPAUSEN for hg%d\n"), FUNCTION_NAME(), p));
            }
        }
    
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
    if (SOC_SBX_STATE(unit)->port_state->port_info != NULL) {
        sal_free(SOC_SBX_STATE(unit)->port_state->port_info);
        SOC_SBX_STATE(unit)->port_state->port_info = NULL;
    }

    if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
        sal_free(SOC_SBX_STATE(unit)->port_state->subport_info);
        SOC_SBX_STATE(unit)->port_state->subport_info = NULL;
    }

    if (SOC_SBX_STATE(unit)->port_state->port_lock != NULL) {
        sal_mutex_destroy(SOC_SBX_STATE(unit)->port_state->port_lock);
        SOC_SBX_STATE(unit)->port_state->port_lock = NULL;
    }

    bcm_sirius_port_congestion_deinit(unit);

    return rv;
}

int
bcm_sirius_port_deinit(int unit)
{
    if (SOC_CONTROL(unit)) {
        if (SOC_SBX_CONTROL(unit)) {
            if (SOC_SBX_STATE(unit)) {
                if (SOC_SBX_STATE(unit)->port_state) {
                    if (SOC_SBX_STATE(unit)->port_state->port_info) {
                        sal_free(SOC_SBX_STATE(unit)->port_state->port_info);
                        SOC_SBX_STATE(unit)->port_state->port_info = NULL;
                    }
                    if (SOC_SBX_STATE(unit)->port_state->subport_info) {
                        sal_free(SOC_SBX_STATE(unit)->port_state->subport_info);
                        SOC_SBX_STATE(unit)->port_state->subport_info = NULL;
                    }
                    if (SOC_SBX_STATE(unit)->port_state->port_lock) {
                        sal_mutex_destroy(SOC_SBX_STATE(unit)->port_state->port_lock);
                        SOC_SBX_STATE(unit)->port_state->port_lock = NULL;
                    }
                } /* if (SOC_SBX_STATE(unit)->port_state) */
            } /* if (SOC_SBX_STATE(unit)) */
        } /* if (SOC_SBX_CONTROL(unit)) */
    } /* if (SOC_CONTROL(unit)) */
    bcm_sirius_port_congestion_deinit(unit);
    return bcm_sbx_port_clear(unit);
}

static int
_bcm_sirius_port_to_serdes(int unit, bcm_port_t port, int *nSi)
{
    int rv = BCM_E_NONE;

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        *nSi = SOC_PORT_OFFSET(unit, port);
        if ((*nSi >= SB_FAB_DEVICE_SIRIUS_LINKS) || (*nSi < 0)) {
            return BCM_E_PORT;
        }
    } else {
        rv = BCM_E_PORT;
    }

    return rv;
}

static int
bcm_sirius_port_congestion_init(int unit)
{
    int rv = BCM_E_NONE;


    if (congestion_info[unit] == NULL) {
        congestion_info[unit] = sal_alloc((sizeof(bcm_sbx_port_congestion_info_t) * 2 *
	      (SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS + SB_FAB_DEVICE_SIRIUS_NUM_OOB_HCFC_PORTS)),
	      "congestion state");
        if (congestion_info[unit] == NULL) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "congestion init: unit %d unable to allocate memory\n"),
                       unit));
            return(BCM_E_MEMORY);
        }
    }

    sal_memset(congestion_info[unit], 0x00, (sizeof(bcm_sbx_port_congestion_info_t) * 2 *
              (SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS + SB_FAB_DEVICE_SIRIUS_NUM_OOB_HCFC_PORTS)));

    /* only SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS will be used. Line HiGig Ports     */
    /* are (0 - 3).                                                                   */ 

    return(rv);
}

static int
bcm_sirius_port_congestion_deinit(int unit)
{
    int rv = BCM_E_NONE;


    if (congestion_info[unit] != NULL) {
        sal_free(congestion_info[unit]);
    }
    congestion_info[unit] = NULL;

    return(rv);
}


/* NOTE: Could be moved later to MAC Driver */
static int
bcm_sirius_port_mac_congestion_set(int unit, int port, soc_higig_e2ecc_hdr_t *e2ecc_hdr,
                                                       soc_higig_e2ecc_hdr_t *e2ecc_mask)
{

    /* HiGig Header */
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_DATA0r(unit, port, e2ecc_hdr->overlay1.words[0]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_DATA1r(unit, port, e2ecc_hdr->overlay1.words[1]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_DATA2r(unit, port, e2ecc_hdr->overlay1.words[2]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_DATA3r(unit, port, e2ecc_hdr->overlay1.words[3]));

    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_MASK0r(unit, port, e2ecc_mask->overlay1.words[0]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_MASK1r(unit, port, e2ecc_mask->overlay1.words[1]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_MASK2r(unit, port, e2ecc_mask->overlay1.words[2]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_MH_MASK3r(unit, port, e2ecc_mask->overlay1.words[3]));

    /* Payload */
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_DATA0r(unit, port, e2ecc_hdr->overlay1.words[4]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_DATA1r(unit, port, e2ecc_hdr->overlay1.words[5]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_DATA2r(unit, port, e2ecc_hdr->overlay1.words[6]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_DATA3r(unit, port, e2ecc_hdr->overlay1.words[7]));

    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_MASK0r(unit, port, e2ecc_mask->overlay1.words[4]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_MASK1r(unit, port, e2ecc_mask->overlay1.words[5]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_MASK2r(unit, port, e2ecc_mask->overlay1.words[6]));
    SOC_IF_ERROR_RETURN(WRITE_XHOL_RX_PKT_MASK3r(unit, port, e2ecc_mask->overlay1.words[7]));
   
    return BCM_E_NONE;
}

static int
bcm_sirius_port_congestion_mode_set(int unit, int port,
				    bcm_port_congestion_config_t *config,
				    int out_of_band)
{
    int rv = BCM_E_NONE;
    uint16    dev_id;
    uint8     rev_id;
    uint32    regval;
    soc_higig_e2ecc_hdr_t e2ecc_hdr;
    soc_higig_e2ecc_hdr_t e2ecc_mask;
    int is_encap_higig;
    uint64 val64=COMPILER_64_INIT(0,0);
    uint64 uuZero = COMPILER_64_INIT(0,0), uuOne = COMPILER_64_INIT(0,1);
    uint32    hcfc_enable = 0;
#if 0
    int i;
#endif /* 0 */

    soc_cm_get_id(unit, &dev_id, &rev_id);

    if (config->flags & BCM_PORT_CONGESTION_CONFIG_E2ECC) {
	/* E2ECC RX */
	SOC_IF_ERROR_RETURN(READ_ES_LL_FC_CONFIGr(unit, &regval));
	soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &regval, FC_MSG_TYPEf, 0);         /* E2ECC */
	SOC_IF_ERROR_RETURN(WRITE_ES_LL_FC_CONFIGr(unit, regval));

	if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == FALSE) {
	    /* determine if MC merge needs to be configured */
	    rv = bcm_sbx_port_any_is_encap_higig(unit, &is_encap_higig);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	    
	    rv = soc_sirius_es_fc_mc_merge_set(unit, is_encap_higig);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	}
	
	/* Formulate the header / mask fields to classify an E2ECC message       */
	/* NOTE: Make sure that the classification also works with SBX headers   */
	/*       This wil require that QID space does not overlap the MGID space */
	
	sal_memset((void *)&e2ecc_hdr, 0, sizeof(soc_higig_e2ecc_hdr_t));
	sal_memset((void *)&e2ecc_mask, 0, sizeof(soc_higig_e2ecc_hdr_t));
	
	/* HiGig Header. For Classifing the packet SRC_MODID/PID is wildcard */
	e2ecc_hdr.overlay1.words[0] =
	    (SOC_HIGIG_START << 24) |                                 /* K.SOP */
	    (((1 << 4) | (config->traffic_class & 0x0F)) << 16) |     /* MC and TC */ 
	    (((config->multicast_id >> 8) & 0xFF) << 8) |             /* MGID (msb) */
	    (config->multicast_id & 0xFF);                            /* MGID (lsb) */
	e2ecc_hdr.overlay1.words[1] = ((config->color & 0x03) << 6);              /* DP */
	
	e2ecc_mask.overlay1.words[0] =
	    ~((0xFF << 24) |                                          /* K.SOP */
	      (((1 << 4) | (0x0F)) << 16) |                             /* MC and TC */
	      (0xFF << 8) |                                             /* MGID (msb) */
	      (0xFF));                                                  /* MGID (lsb) */
	e2ecc_mask.overlay1.words[1] = ~(0x03 << 6);                              /* DP */
	e2ecc_mask.overlay1.words[2] = 0xFFFFFFFF;                                /* Reserved */
	e2ecc_mask.overlay1.words[3] = 0xFFFFFFFF;                                /* Reserved */
	
	
	/* Payload. For Classifing the packet MACSA is wildcard.          */
	/* src_mac[0] is the most significant byte (the byte containing   */
	/* multicast/unicast field) and is programmed in the MACDA[47:40] */
	/* field of E2ECC message.                                        */
	e2ecc_hdr.overlay1.words[4] =                                             /* MACDA */
	    (config->dest_mac[0] << 24)|
	    (config->dest_mac[1] << 16) |
	    (config->dest_mac[2] << 8) |
	    (config->dest_mac[3]);
	e2ecc_hdr.overlay1.words[5] =                                             /* MACDA */
	    (config->dest_mac[4]  << 24) |
	    (config->dest_mac[5] << 16);
	e2ecc_hdr.overlay1.words[7] =
	    (((config->ethertype >> 8) & 0xFF) << 24) |               /* Ethertype (msb) */
	    ((config->ethertype & 0xFF) << 16) |                      /* Ethertype (lsb) */
	    (((config->opcode >> 8) & 0xFF) << 8) |                   /* Opcode (msb) */
	    (config->opcode & 0xFF);                                  /* Opcode (lsb) */
	
	
	e2ecc_mask.overlay1.words[4] = ~(0xFFFFFFFF);                             /* MACDA */
	e2ecc_mask.overlay1.words[5] = ~(0xFFFF0000);                             /* MACDA */
	e2ecc_mask.overlay1.words[6] = 0xFFFFFFFF;                                /* MACSA */
	e2ecc_mask.overlay1.words[7] = ~(0xFFFFFFFF);                             /* Ethertype/Opcode */
	
	
#if 0
	/* HiGig Header. For Classifing the packet SRC_MODID/PID is wildcard */
	e2ecc_hdr.overlay0.bytes[0] = SOC_HIGIG_START;                            /* K.SOP */
	e2ecc_hdr.overlay0.bytes[1] = (1 << 4) | (config->traffic_class & 0x0F);  /* MC and TC */
	e2ecc_hdr.overlay0.bytes[2] = config->multicast_id & 0xFF;                /* MGID (lsb) */
	e2ecc_hdr.overlay0.bytes[3] = (config->multicast_id >> 8) & 0xFF;         /* MGID (msb) */
	e2ecc_hdr.overlay0.bytes[7] = (config->color & 0x03) << 6;                /* DP */
	
	e2ecc_mask.overlay0.bytes[0] = ~(0xFF);                                   /* K.SOP */;
	e2ecc_mask.overlay0.bytes[1] = ~((1 << 4) | (0x0F));                      /* MC and TC */
	e2ecc_mask.overlay0.bytes[2] = ~(0xFF);                                   /* MGID (lsb) */
	e2ecc_mask.overlay0.bytes[3] = ~(0xFF);                                   /* MGID (msb) */
	e2ecc_mask.overlay0.bytes[4] = 0xFF;
	e2ecc_mask.overlay0.bytes[5] = 0xFF;
	e2ecc_mask.overlay0.bytes[6] = 0xFF;
	e2ecc_mask.overlay0.bytes[7] = ~((0x03 << 6));                            /* DP */
	for (i = 0; i < 8; i++) {                                                 /* Reserved */
	    e2ecc_mask.overlay0.bytes[(8 + i)] = 0xFF;
	}
	
	
	/* Payload. For Classifing the packet MACSA is wildcard */
	for (i = 0; i < sizeof(bcm_mac_t); i++) {                                 /* MACDA */
	    e2ecc_hdr.overlay0.bytes[(16 + i)] = config->src_mac[0];
	}
	e2ecc_hdr.overlay0.bytes[28] = config->ethertype & 0xFF;                  /* Ethertype (lsb) */
	e2ecc_hdr.overlay0.bytes[29] = (config->ethertype >> 8) & 0xFF;           /* Ethertype (msb) */
	e2ecc_hdr.overlay0.bytes[30] = config->opcode & 0xFF;                     /* Opcode (lsb) */
	e2ecc_hdr.overlay0.bytes[31] = (config->opcode >> 8) & 0xFF;              /* Opcode (msb) */
	
	for (i = 0; i < sizeof(bcm_mac_t); i++) {                                 /* MACDA */
	    e2ecc_mask.overlay0.bytes[(16 + i)] = ~(0xFF);
	}
	for (i = 0; i < sizeof(bcm_mac_t); i++) {                                 /* MACSA */
	    e2ecc_mask.overlay0.bytes[(22 + i)] = 0xFF;
	}
	e2ecc_mask.overlay0.bytes[28] = ~(0xFF);                                  /* Ethertype (lsb) */
	e2ecc_mask.overlay0.bytes[29] = ~(0xFF);                                  /* Ethertype (msb) */
	e2ecc_mask.overlay0.bytes[30] = ~(0xFF);                                  /* Opcode (lsb) */
	e2ecc_mask.overlay0.bytes[31] = ~(0xFF);                                  /* Opcode (msb) */
#endif /* 0 */
	
	rv = bcm_sirius_port_mac_congestion_set(unit, port, &e2ecc_hdr, &e2ecc_mask);
    } else if (config->flags & BCM_PORT_CONGESTION_CONFIG_HCFC) {
	if (out_of_band) {
	    /* out of band HCFC port associated with xp0/1 */
	    port += SOC_PORT_MIN(unit, hg);
	}

	if (config->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
	    /* HCFC TX */
	    
	    /* config xport/bigmac to enable HCFC TX
	     * CRC needs to be ignored since Trident calculated CRC in different endian
	     */
	    SOC_IF_ERROR_RETURN(READ_MAC_HCFC_CTRLr(unit, port, &val64));
            soc_reg64_field_set(unit, MAC_HCFC_CTRLr, &val64, HCFC_TX_ENf, uuOne);
            soc_reg64_field_set(unit, MAC_HCFC_CTRLr, &val64, HCFC_OUT_OF_BAND_MODEf,
				(out_of_band?uuOne:uuZero));
	    soc_reg64_field_set(unit, MAC_HCFC_CTRLr, &val64, HCFC_IGNORE_CRCf, uuOne);
            SOC_IF_ERROR_RETURN(WRITE_MAC_HCFC_CTRLr(unit, port, val64));

	    /* config rb, do we need to set the HCFC message fields? */
	    SOC_IF_ERROR_RETURN(READ_RB_FC_E2ECC_HCFC_CONFIGr(unit, &regval));
	    soc_reg_field_set(unit, RB_FC_E2ECC_HCFC_CONFIGr, &regval, HCFC_GLOBAL_MODEf,
			      (out_of_band?1:0));
	    soc_reg_field_set(unit, RB_FC_E2ECC_HCFC_CONFIGr, &regval, XP0_CFG_PROTOCOLf, 2);
	    soc_reg_field_set(unit, RB_FC_E2ECC_HCFC_CONFIGr, &regval, XP1_CFG_PROTOCOLf, 2);
	    soc_reg_field_set(unit, RB_FC_E2ECC_HCFC_CONFIGr, &regval, XP2_CFG_PROTOCOLf, 2);
	    soc_reg_field_set(unit, RB_FC_E2ECC_HCFC_CONFIGr, &regval, XP3_CFG_PROTOCOLf, 2);
	    SOC_IF_ERROR_RETURN(WRITE_RB_FC_E2ECC_HCFC_CONFIGr(unit, regval));	    
	    
	    if (rev_id >= BCM88230_C0_REV_ID) {
		/* config qm, channel_base 0, 16 ports, HCFC */
		SOC_IF_ERROR_RETURN(READ_QM_HCFC_CONFIG0r(unit, &regval));
		hcfc_enable = soc_reg_field_get(unit, QM_HCFC_CONFIG0r, regval, HCFC_ENABLEf);
		hcfc_enable = 1 << (port - SOC_PORT_MIN(unit, hg));
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC_ENABLEf, (hcfc_enable&0x3));
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC_FWDf, 0);
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC_LLFCf, 0);
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC_MSG_TYPEf, 1);
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC0_CHANNEL_BASEf, 0);
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC1_CHANNEL_BASEf, 0);
		soc_reg_field_set(unit, QM_HCFC_CONFIG0r, &regval, HCFC_COMMON_MODE_PORT_COUNTf, 0xF);
		SOC_IF_ERROR_RETURN(WRITE_QM_HCFC_CONFIG0r(unit, regval));	    

		/* config to send out HCFC continuously, supporting OOB HCFC only anyway */
		SOC_IF_ERROR_RETURN(READ_QM_HCFC_CONFIG1r(unit, &regval));
		soc_reg_field_set(unit, QM_HCFC_CONFIG1r, &regval, HCFC_CONSTANT_MODEf, 1);
		soc_reg_field_set(unit, QM_HCFC_CONFIG1r, &regval, HCFC_LIMIT_CNTf, 200);
		SOC_IF_ERROR_RETURN(WRITE_QM_HCFC_CONFIG1r(unit, regval));
	    }
	}

	if (config->flags & BCM_PORT_CONGESTION_CONFIG_RX) {
	    /* HCFC RX */

	    /* config xport/bigmac to enable HCFC RX
	     * CRC needs to be ignored since Trident calculated CRC in different endian
	     */
	    SOC_IF_ERROR_RETURN(READ_MAC_HCFC_CTRLr(unit, port, &val64));
            soc_reg64_field_set(unit, MAC_HCFC_CTRLr, &val64, HCFC_RX_ENf, uuOne);
            soc_reg64_field_set(unit, MAC_HCFC_CTRLr, &val64, HCFC_OUT_OF_BAND_MODEf,
				(out_of_band?uuOne:uuZero));
	    soc_reg64_field_set(unit, MAC_HCFC_CTRLr, &val64, HCFC_IGNORE_CRCf, uuOne);
            SOC_IF_ERROR_RETURN(WRITE_MAC_HCFC_CTRLr(unit, port, val64));

	    /* config number of bits for a port in the HCFC message */
	    SOC_IF_ERROR_RETURN(READ_ES_LL_FC_CONFIGr(unit, &regval));
	    switch (config->port_bits) {
		case 1:
		    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &regval, FC_HC_MODEf, 3);
		    break;
		case 2:
		    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &regval, FC_HC_MODEf, 2);
		    break;
		case 4:
		    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &regval, FC_HC_MODEf, 1);
		    break;
		case 8:
		    /* Since flow_control_translation_table can only translate into 4 bits result
		     * we have to process 8 bits in 2 rounds, 4 bits for each round. FC_HC_MODE 
		     * need to be 1 instead of 0.
		     */
		    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &regval, FC_HC_MODEf, 1);
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_PORT,
		              (BSL_META_U(unit,
		                          "HCFC reception %d bits per port not supported on unit %d\n"),
		               config->port_bits, unit));
		    return BCM_E_PARAM;
	    }
	    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &regval, FC_MSG_TYPEf, 1);  /* HCFC */
	    SOC_IF_ERROR_RETURN(WRITE_ES_LL_FC_CONFIGr(unit, regval));	    
	}
    } else if (config->flags & BCM_PORT_CONGESTION_CONFIG_SAFC) {
	/* SAFC */
	
	/* bigmac SAFC */
	
	SOC_IF_ERROR_RETURN(READ_MAC_TXLLFCCTRLr(unit, port, &val64));
	soc_reg64_field_set(unit, MAC_TXLLFCCTRLr, &val64, LLFC_ENf, uuOne);
	SOC_IF_ERROR_RETURN(WRITE_MAC_TXLLFCCTRLr(unit, port, val64));
	
	/* xport SAFC */
	SOC_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &regval));
	soc_reg_field_set(unit, XPORT_CONFIGr, &regval, LLFC_ENf, 1);
	SOC_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, regval));

	/* qm SAFC */
	if (soc_feature(unit, soc_feature_priority_flow_control) &&
	    !soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    SOC_IF_ERROR_RETURN(READ_QM_LLFC_CONFIGr(unit, &regval));
	    soc_reg_field_set(unit, QM_LLFC_CONFIGr, &regval, LLFC_ENABLEf, 1);
	    SOC_IF_ERROR_RETURN(WRITE_QM_LLFC_CONFIGr(unit, regval));
	}
    }

    return(rv);
}

int
bcm_sirius_port_congestion_config_set(int unit, bcm_gport_t port,
				      bcm_port_congestion_config_t *config, void *info)
{
    int rv = BCM_E_NONE;
    bcm_module_t my_modid, modid;
    bcm_port_t port_nbr;
    int        congestion_id;
    int        out_of_band = FALSE;

    rv = bcm_stk_my_modid_get(unit, &my_modid);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to get module id, unit %d\n"), unit));
        return(rv);
    }

    /* consistency check */
    if (BCM_GPORT_IS_MODPORT(port)) {
        modid = BCM_GPORT_MODPORT_MODID_GET(port);
        port_nbr = BCM_GPORT_MODPORT_PORT_GET(port);
    }
    else if (BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
        port_nbr = BCM_GPORT_EGRESS_MODPORT_PORT_GET(port);
    }
    else if (BCM_GPORT_IS_CONGESTION(port)) {
        modid = my_modid;
	port_nbr = BCM_GPORT_CONGESTION_GET(port);
    }	
    else {
        modid = my_modid;
        port_nbr = port;
    }

    if (modid != my_modid) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "module id is not this unit, unit %d, modid: %d expected modid: %d\n"), unit, modid, my_modid));
        return(BCM_E_UNAVAIL);
    }

    if (BCM_GPORT_IS_CONGESTION(port)) {
	if ((port_nbr < 0) || (port_nbr > 1)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "Invalid Out of band HCFC port, unit %d, port %d\n"), unit, port_nbr));
	    return(BCM_E_PARAM);
	}
	/* separate RX/TX config if required */
	congestion_id = (port_nbr + SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS) * 2;
	if (config->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
	    congestion_id++;
	}
	out_of_band = TRUE;
    } else {
	if (!IS_HG_PORT(unit, port_nbr)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "Invalid HiGig port, unit %d, port %d\n"), unit, port_nbr));
	    return(BCM_E_PARAM);
	}
	
	/* separate RX/TX config if required */
	congestion_id = (port_nbr - SOC_PORT_MIN(unit, hg)) * 2;
	if (config->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
	    congestion_id++;
	}
    }

    if ( (config->flags & BCM_PORT_CONGESTION_CONFIG_MAX_SPLIT4) ||
         (config->flags & BCM_PORT_CONGESTION_CONFIG_DESTMOD_FLOW_CONTROL) ) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Unsupported flags Option specified, unit %d, port %d flag 0x%x\n"), unit, port_nbr, config->flags));
        return(BCM_E_PARAM);
    }
    if (!((config->flags & BCM_PORT_CONGESTION_CONFIG_E2ECC) ||
	  (config->flags & BCM_PORT_CONGESTION_CONFIG_HCFC) ||
	  (config->flags & BCM_PORT_CONGESTION_CONFIG_SAFC)) ) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Need to specify Flow Control Mode, unit %d, port %d flag 0x%x\n"), unit, port_nbr, config->flags));
        return(BCM_E_PARAM);
    }
    if (!((config->flags & BCM_PORT_CONGESTION_CONFIG_RX) ||
	  (config->flags & BCM_PORT_CONGESTION_CONFIG_TX))) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Need to specify Flow Control Direction, unit %d, port %d flag 0x%x\n"), unit, port_nbr, config->flags));
        return(BCM_E_PARAM);
    }

    /* HiGig configuration */
    rv = bcm_sirius_port_congestion_mode_set(unit, port_nbr, config, out_of_band);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Error configuring congestion HiGig Port, unit %d, port %d\n"), unit, port_nbr));
        return(rv);
    }

    if (config->flags & BCM_PORT_CONGESTION_CONFIG_E2ECC) {
	/* cosq configuration */
	rv = bcm_sirius_cosq_congestion_set(unit, port, (congestion_info[unit] + congestion_id), config);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "Error configuring congestion (cosq), unit %d, port %d\n"), unit, port_nbr));
	    return(rv);
	}
    }

    /* update state */
    (congestion_info[unit] + congestion_id)->config = (*config);

    return rv;
}

int
bcm_sirius_port_congestion_config_get(int unit, bcm_gport_t port,
                                           bcm_port_congestion_config_t *config, void *info)
{
    int rv = BCM_E_NONE;
    bcm_module_t my_modid, modid;
    bcm_port_t port_nbr;
    int        congestion_id;


    rv = bcm_stk_my_modid_get(unit, &my_modid);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "unable to get module id, unit %d\n"), unit));
        return(rv);
    }

    /* consistency check */
    if (BCM_GPORT_IS_MODPORT(port)) {
        modid = BCM_GPORT_MODPORT_MODID_GET(port);
        port_nbr = BCM_GPORT_MODPORT_PORT_GET(port);
    }
    else if (BCM_GPORT_IS_EGRESS_MODPORT(port)) {
        modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(port);
        port_nbr = BCM_GPORT_EGRESS_MODPORT_PORT_GET(port);
    }
    else if (BCM_GPORT_IS_CONGESTION(port)) {
        modid = my_modid;
	port_nbr = BCM_GPORT_CONGESTION_GET(port);
    }	
    else {
        modid = my_modid;
        port_nbr = port;
    }

    if (modid != my_modid) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "module id is not this unit, unit %d, modid: %d expected modid: %d\n"), unit, modid, my_modid));
        return(BCM_E_UNAVAIL);
    }

    if (BCM_GPORT_IS_CONGESTION(port)) {
	if ((port_nbr < 0) || (port_nbr > 1)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "Invalid Out of band HCFC port, unit %d, port %d\n"), unit, port_nbr));
	    return(BCM_E_PARAM);
	}

	/* separate RX/TX config if required */
	congestion_id = (port_nbr + SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS) * 2;
	if (config->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
	    congestion_id++;
	}
    } else {
	if (!IS_HG_PORT(unit, port_nbr)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "Invalid HiGig port, unit %d, port %d\n"), unit, port_nbr));
	    return(BCM_E_PARAM);
	}
	
	/* separate RX/TX config if required */
	congestion_id = (port_nbr - SOC_PORT_MIN(unit, hg)) * 2;
	if (config->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
	    congestion_id++;
	}
    }

    if (!((config->flags & BCM_PORT_CONGESTION_CONFIG_RX) ||
	  (config->flags & BCM_PORT_CONGESTION_CONFIG_TX))) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "Need to specify Flow Control Direction, unit %d, port %d flag 0x%x\n"), unit, port_nbr, config->flags));
        return(BCM_E_PARAM);
    }

    /* update state */
    (*config) = (congestion_info[unit] + congestion_id)->config;

    return rv;
}

int
bcm_sirius_port_pause_get(int unit, bcm_port_t port, int *pause_tx, int *pause_rx)
{
    int rv = BCM_E_UNAVAIL;

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        return BCM_E_NONE;
    } else if (IS_GX_PORT(unit, port)) {
        rv = MAC_PAUSE_GET(SIRIUS_PORT(unit, port).p_mac, 
                           unit, 
                           port, 
                           pause_tx, 
                           pause_rx);
    }

    return rv;
}

int
bcm_sirius_port_pause_set(int unit, bcm_port_t port, int pause_tx, int pause_rx)
{
    int         rv  = BCM_E_UNAVAIL;

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
        return BCM_E_NONE;
    } else if (IS_GX_PORT(unit, port)) {
        rv = MAC_PAUSE_SET(SIRIUS_PORT(unit, port).p_mac, 
                           unit, 
                           port, 
                           pause_tx, 
                           pause_rx);
    }

    return rv;
}

int
bcm_sirius_port_update(int unit, bcm_port_t port, int link)
{
    int                 rv;
    int                 duplex = 0, an, an_done;
    soc_port_if_t       pif;

    if (!IS_HG_PORT(unit, port) && !IS_XE_PORT(unit, port)) {
         return BCM_E_UNAVAIL;
    }

    if (!link) {
        /* PHY is down.  Disable the MAC. */

        rv = (MAC_ENABLE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, FALSE));
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_ENABLE_SET FALSE rv=%d\n"),
                      unit, port, rv));
            return rv;
        }

        /* PHY link down event */
        rv = (soc_phyctrl_linkdn_evt(unit, port));
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d soc_phyctrl_linkdn_evt rv=%d\n"),unit, port, rv));
            return rv;
        }

        return BCM_E_NONE;
    }

    /* PHY link up event may not be support by all PHY driver.
     * Just ignore it if not supported */
    rv = (soc_phyctrl_linkup_evt(unit, port));
    if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d soc_phyctrl_linkup_evt rv=%d\n"),unit, port, rv));
        return rv;
    }

    if (IS_HG_PORT(unit, port)) {
        duplex = 1;
    }

    rv = (soc_phyctrl_interface_get(unit, port, &pif));
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d phyctrl_interface_get rv=%d\n"),
                  unit, port, rv));
        return rv;
    }
    rv = (MAC_INTERFACE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, pif));
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d MAC_INTERFACE_GET rv=%d\n"),
                  unit, port,rv));
        return rv;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_auto_negotiate_get(unit, port, &an, &an_done));

    /*
     * If autonegotiating, check the negotiated PAUSE values, and program
     * MACs accordingly.
     */

    if (an) {
        bcm_port_ability_t      remote_advert, local_advert;
        int                     tx_pause, rx_pause;

        rv = soc_phyctrl_ability_advert_get(unit, port, &local_advert);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d soc_phyctrl_adv_local_get rv=%d\n"),
                      unit, port, rv));
            return rv;
        }
        rv = soc_phyctrl_ability_remote_get(unit, port, &remote_advert);
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d soc_phyctrl_adv_remote_get rv=%d\n"),
                      unit, port, rv));
            return rv;
        }

        /*
         * IEEE 802.3 Flow Control Resolution.
         * Please see $SDK/doc/pause-resolution.txt for more information.
         */

        if (duplex) {
             tx_pause =
                         ((remote_advert.pause & SOC_PA_PAUSE_RX) &&
                          (local_advert.pause & SOC_PA_PAUSE_RX)) ||
                         ((remote_advert.pause & SOC_PA_PAUSE_RX) &&
                          !(remote_advert.pause & SOC_PA_PAUSE_TX) &&
                          (local_advert.pause & SOC_PA_PAUSE_TX));

                     rx_pause =
                         ((remote_advert.pause & SOC_PA_PAUSE_RX) &&
                          (local_advert.pause & SOC_PA_PAUSE_RX)) ||
                         ((local_advert.pause & SOC_PA_PAUSE_RX) &&
                          (remote_advert.pause & SOC_PA_PAUSE_TX) &&
                          !(local_advert.pause & SOC_PA_PAUSE_TX));
        } else {
            rx_pause = tx_pause = 0;
        }

        rv = (MAC_PAUSE_SET(SIRIUS_PORT(unit, port).p_mac,
                           unit, port, tx_pause, rx_pause));
        if (BCM_FAILURE(rv)) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "u=%d p=%d MAC_PAUSE_SET rv=%d\n"),
                      unit, port, rv));
            return rv;
        }
    }
    /* Enable the MAC. */
    rv =  (MAC_ENABLE_SET(SIRIUS_PORT(unit, port).p_mac, unit, port, TRUE));
    if (BCM_FAILURE(rv)) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "u=%d p=%d MAC_ENABLE_SET TRUE rv=%d\n"),
                  unit, port, rv));
    }

        return rv;
}

int
bcm_sirius_port_scheduler_get(int unit,
			      bcm_gport_t gport,
			      int *scheduler_level,
			      int *scheduler_node)
{
    int port, sc_id, intf, eg_n = -1, multipath;
    bcm_sbx_subport_info_t *sp_info;
    int rv = BCM_E_NONE;    

    if (BCM_GPORT_IS_CHILD(gport) || BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	/* ingress or egress scheduler for higig subport */
	if (BCM_GPORT_IS_CHILD(gport)) {
	    port = BCM_GPORT_CHILD_PORT_GET(gport);
	} else {
	    port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
	}

	if (port >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, child gport 0x%x out of range, unit %d\n"),
                       FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
	if (sp_info->valid == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid child gport 0x%x, unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}	

	/* ingress scheduler for higig subport */
	if (BCM_GPORT_IS_CHILD(gport)) {
	    if ((sp_info->flags & SBX_SUBPORT_FLAG_ON_TS) == 0) {
		LOG_ERROR(BSL_LS_BCM_PORT,
		          (BSL_META_U(unit,
		                      "ERROR: %s, child gport 0x%x has no ingress scheduler resource allocated, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		return BCM_E_PARAM;	    
	    }
	    *scheduler_level = sp_info->ts_scheduler_level;
	    *scheduler_node = sp_info->ts_scheduler_node;
	} else {
	    if ((sp_info->flags & SBX_SUBPORT_FLAG_ON_ES) == 0) {
		LOG_ERROR(BSL_LS_BCM_PORT,
		          (BSL_META_U(unit,
		                      "ERROR: %s, child gport 0x%x has no egress scheduler resource allocated, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		return BCM_E_PARAM;	    
	    }

	    if ((sp_info->flags & SBX_SUBPORT_FLAG_IN_TRUNK) == 0) {
		*scheduler_level = SIRIUS_ES_LEVEL_CHANNEL;
		*scheduler_node = sp_info->es_scheduler_level2_node;
	    } else {
		/* the subport requested is trunked, if caller specifies the
		 * SIRIUS_ES_LEVEL_GROUP_SHAPER, then return the group shaper id
		 * otherwise still return the true level/node
		 */
		if (*scheduler_level == SIRIUS_ES_LEVEL_GROUP_SHAPER) {
		    if (sp_info->group_shaper > 0) {
			*scheduler_node = sp_info->group_shaper;
		    } else {
			LOG_ERROR(BSL_LS_BCM_PORT,
			          (BSL_META_U(unit,
			                      "ERROR: %s, child gport 0x%x is in trunk, but no group shaper resources allocated, unit %d\n"),
			           FUNCTION_NAME(), gport, unit));
			return BCM_E_PARAM;
		    }
		} else {
		    *scheduler_level = SIRIUS_ES_LEVEL_CHANNEL;
		    *scheduler_node = sp_info->es_scheduler_level2_node;
		}
	    }
	}
    } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	/* egress scheduler for fifo groups */
	port = -1;
	if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, NULL) != BCM_E_NONE) { 
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, gport 0x%x not allocated on unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}
	
	if ((port < 0) || (port >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress group gport 0x%x out of range, unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
	if (sp_info->valid == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid egress group gport 0x%x, unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}	

	/* return base level 0 fifo of the fifo group */
	*scheduler_level = SIRIUS_ES_LEVEL_FIFO;
	*scheduler_node = sp_info->egroup[eg_n].es_scheduler_level0_node;

    } else if (BCM_GPORT_IS_MODPORT(gport)) {
	/* ingress scheduler for higig port */
	port = BCM_GPORT_MODPORT_PORT_GET(gport);
	if (!SOC_PORT_VALID(unit, port)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid gport 0x%x, unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}

	*scheduler_level = 7;

	if (IS_GX_PORT(unit, port)) {
	    /* Higig PORT */
	    intf = SOC_PORT_OFFSET(unit,port);
	    if (intf >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) {
		LOG_ERROR(BSL_LS_BCM_PORT,
		          (BSL_META_U(unit,
		                      "ERROR: %s, gport 0x%x not line-side Higig port, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		return BCM_E_PARAM;
	    }
	    *scheduler_node = intf + SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_HG0_NODE;
	} else if (IS_CPU_PORT(unit, port)) {
	    /* CPU port */
	    *scheduler_node = SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_CPU_NODE;
	} else if (IS_REQ_PORT(unit, port)) {
	    /* Requeue port */
	    intf = SOC_PORT_OFFSET(unit,port);
	    if (intf >= SB_FAB_DEVICE_SIRIUS_NUM_REQUEUE_PORTS) {
		LOG_ERROR(BSL_LS_BCM_PORT,
		          (BSL_META_U(unit,
		                      "ERROR: %s, gport 0x%x not valid requeue port, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		return BCM_E_PARAM;
	    }
	    *scheduler_node = intf + SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ0_NODE;
	} else {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, No ingress scheduler resource for gport 0x%x, unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	/* egress scheduler for higig port */
	*scheduler_level = SIRIUS_ES_LEVEL_INTERFACE;
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);

	if (IS_GX_PORT(unit, port)) {
	    /* Higig PORT */
	    intf = SOC_PORT_OFFSET(unit,port);
	    if (intf >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) {
		LOG_ERROR(BSL_LS_BCM_PORT,
		          (BSL_META_U(unit,
		                      "ERROR: %s, gport 0x%x not line-side Higig port, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		return BCM_E_PARAM;
	    }
	    *scheduler_node = intf + SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_HG0_NODE;
	} else if (IS_CPU_PORT(unit, port)) {
	    /* CPU port */
	    *scheduler_node = SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_CPU_NODE;
	} else if (IS_REQ_PORT(unit, port)) {
	    /* Requeue port */
	    intf = SOC_PORT_OFFSET(unit,port);
	    if (intf >= SB_FAB_DEVICE_SIRIUS_NUM_REQUEUE_PORTS) {
		LOG_ERROR(BSL_LS_BCM_PORT,
		          (BSL_META_U(unit,
		                      "ERROR: %s, gport 0x%x not valid requeue port, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		return BCM_E_PARAM;
	    }
	    *scheduler_node = intf + SB_FAB_DEVICE_SIRIUS_EGRESS_SCHEDULER_RQ0_NODE;
	} else {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, No egress scheduler resource for gport 0x%x, unit %d\n"),
	               FUNCTION_NAME(), gport, unit));
	    return BCM_E_PARAM;
	}
    } else if (BCM_GPORT_IS_SCHEDULER(gport)) {
	/* scheduler gport */
	sc_id = BCM_INT_SBX_SCHEDULER_ID_GET(gport);

	if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) {
	    if ((sc_id < 0) || (sc_id >= SOC_SBX_CFG(unit)->num_egress_scheduler) ||
		(SOC_SBX_STATE(unit)->egress_scheduler_state == NULL) ||
		(SOC_SBX_STATE(unit)->egress_scheduler_state[sc_id].in_use == FALSE)) {
		return BCM_E_PARAM;
	    }
	    *scheduler_level = SOC_SBX_STATE(unit)->egress_scheduler_state[sc_id].level;
	    *scheduler_node = SOC_SBX_STATE(unit)->egress_scheduler_state[sc_id].node;
	} else {
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
		/* physical handle */
		*scheduler_level = BCM_INT_SBX_SCHEDULER_LEVEL_GET(gport);
		*scheduler_node =  BCM_INT_SBX_SCHEDULER_NODE_GET(gport);
		if ((*scheduler_level <= 0) || 
		    (*scheduler_level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS ) ||
		    (*scheduler_node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[*scheduler_level])) {
		    LOG_ERROR(BSL_LS_BCM_PORT,
		              (BSL_META_U(unit,
		                          "ERROR: %s, No ingress scheduler resource for gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), gport, unit));
		    return BCM_E_PARAM;
		}
	    } else {
		/* logical handle */
		if ((sc_id < 0) || (sc_id >= SOC_SBX_CFG(unit)->num_ingress_scheduler) ||
		    (SOC_SBX_STATE(unit)->ingress_scheduler_state == NULL) ||
		    (SOC_SBX_STATE(unit)->ingress_scheduler_state[sc_id].in_use == FALSE)) {
		    return BCM_E_PARAM;
		}
		*scheduler_level = SOC_SBX_STATE(unit)->ingress_scheduler_state[sc_id].level;
		*scheduler_node = SOC_SBX_STATE(unit)->ingress_scheduler_state[sc_id].node;
	    }
	}
    } else if (BCM_COSQ_GPORT_IS_MULTIPATH(gport)) {
	/* multipath gport */
	multipath = BCM_INT_SBX_MULTIPATH_ID_GET(gport);

	if (BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport)) {
	    if ((multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_egress_multipath) ||
		(SOC_SBX_STATE(unit)->egress_multipath_state == NULL) ||
		(SOC_SBX_STATE(unit)->egress_multipath_state[multipath].in_use == FALSE)) {
		return BCM_E_PARAM;
	    }
	    *scheduler_level = SB_FAB_DEVICE_GROUP_SHAPER_LEVEL;
	    *scheduler_node = SOC_SBX_STATE(unit)->egress_multipath_state[multipath].node;
	} else {
	    if ((multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_ingress_multipath) ||
		(SOC_SBX_STATE(unit)->ingress_multipath_state == NULL) ||
		(SOC_SBX_STATE(unit)->ingress_multipath_state[multipath].in_use == FALSE)) {
		return BCM_E_PARAM;
	    }
	    *scheduler_level = SOC_SBX_STATE(unit)->ingress_multipath_state[multipath].level;
	    *scheduler_node = SOC_SBX_STATE(unit)->ingress_multipath_state[multipath].node;
	}
    } else {
	*scheduler_level = -1;
	*scheduler_node = -1;
	rv = BCM_E_PARAM;
    }

    return rv;
}

int
bcm_sirius_port_is_egress_multicast(int unit,
                                    bcm_gport_t gport,
                                    bcm_cos_queue_t cosq,
                                    int *is_multicast)
{
    int  rv = BCM_E_NONE;
    int  port;

    (*is_multicast) = FALSE;

    if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
    }
    else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        port = BCM_GPORT_EGRESS_GROUP_GET(gport);
    }
    else {
        rv = BCM_E_PARAM;
        return(rv);
    }

    /* check for internal multicast port */
    if ((port >= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0) &&
                                                  (port <= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE3)) {
        (*is_multicast) = TRUE;
    }

    return(rv);
}

int
bcm_sirius_port_egress_multicast_scheduler_get(int unit,
                                               bcm_gport_t gport,
                                               bcm_cos_queue_t cosq,
                                               int *scheduler_level,
                                               int *nbr_scheduler_nodes,
                                               int *scheduler_nodes)
{
    int                     rv = BCM_E_NONE;
    int                     port;
    bcm_sbx_subport_info_t *sp_info = NULL;


    if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
	LOG_ERROR(BSL_LS_BCM_PORT,
	          (BSL_META_U(unit,
	                      "ERROR: %s, scheduler setting not supported, gport 0x%x, unit %d\n"),
	           FUNCTION_NAME(), gport, unit));
        return(BCM_E_UNAVAIL);
    }
    else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        port = BCM_GPORT_EGRESS_GROUP_GET(gport);
        (*scheduler_level) = SIRIUS_ES_LEVEL_FIFO;
    }
    else {
        rv = BCM_E_PARAM;
        return(rv);
    }

    if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
	if ((sp_info == NULL) || (sp_info->valid == FALSE)) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_group %d, unit %d\n"),
	               FUNCTION_NAME(), port, unit));
	    return(BCM_E_PARAM);
	}
        if (cosq >= sp_info->egroup[0].num_fifos) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid cos %d, noFifos %d unit %d\n"),
	               FUNCTION_NAME(),
	               port, sp_info->egroup[0].num_fifos, unit));
	    return(BCM_E_PARAM);
        }
    }

    return(rv);
}

int
bcm_sirius_port_egress_multicast_group_get(int unit,
                                           bcm_gport_t gport,
                                           bcm_cos_queue_t cosq,
                                           int *nbr_fifos,
                                           int *nbr_egress_groups,
                                           int *egress_groups)
{
    int                     rv = BCM_E_NONE;
    int                     port, higig_port, i, j;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_module_t            modid;
    bcm_trunk_t             tid;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;
    bcm_module_t mod;
    bcm_port_t p;


    if (!BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	LOG_ERROR(BSL_LS_BCM_PORT,
	          (BSL_META_U(unit,
	                      "ERROR: %s, scheduler setting not supported, gport 0x%x, unit %d\n"),
	           FUNCTION_NAME(), gport, unit));
	return(BCM_E_PARAM);
    }

    port = BCM_GPORT_EGRESS_GROUP_GET(gport);
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
    if ((sp_info == NULL) || (sp_info->valid == FALSE)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: %s, invalid fabric_group %d, unit %d\n"),
                   FUNCTION_NAME(), port, unit));
	return(BCM_E_PARAM);
    }
    if (cosq >= sp_info->egroup[0].num_fifos) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: %s, invalid cos %d, noFifos %d unit %d\n"),
                   FUNCTION_NAME(),
                   port, sp_info->egroup[0].num_fifos, unit));
        return(BCM_E_PARAM);
    }
    if (!BCM_GPORT_IS_MODPORT(sp_info->parent_gport)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: %s, egress_group: 0x%x parent 0x%x is not of type ModPort\n"),
                   FUNCTION_NAME(), gport, sp_info->parent_gport));
        return(BCM_E_PARAM);
    }

    higig_port = BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport);
    if (!IS_HG_PORT(unit, higig_port)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "ERROR: %s, parent 0x%x, is not a HiGig interface\n"),
                   FUNCTION_NAME(), sp_info->parent_gport));
        return(BCM_E_PARAM);
    }

    rv = bcm_sirius_stk_modid_get(unit, &modid);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s, failed to get modid, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(rv);
    }

    memberCount = 0; /* for not found case */
    rv = bcm_sirius_trunk_find_and_get(unit,
                                       modid,
                                       sp_info->parent_gport,
                                       &tid,
                                       &trunkInfo,
                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                       &(trunkMembers[0]),
                                       &memberCount);
    if (rv == BCM_E_NOT_FOUND) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s, No HiGig Trunk membership, HiGig Port: %d, HiGig Gport: 0x%x\n"),
                   FUNCTION_NAME(), higig_port, sp_info->parent_gport));
        return(rv);
    }
    else if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "%s, error in getting HiGig Trunk membership, HiGig Port: %d, HiGig Gport: 0x%x\n"),
                   FUNCTION_NAME(), higig_port, sp_info->parent_gport)); 
        return(rv);
    }

    (*nbr_fifos) = sp_info->egroup[0].num_fifos;
    (*nbr_egress_groups) = 0;
    for (i = 0, j = 0; i < memberCount; i++) {
        if (!BCM_GPORT_IS_SET(trunkMembers[i].gport)) {
            mod = modid;
            p = trunkMembers[i].gport;
        } else if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
            mod = BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport);
            p = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
            mod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport);
            p = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
        } else {
            continue;
        }
        if (mod != modid) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "%s HiGig Port (%d) on another module (%d) expected module (%d)\n"), 
                       FUNCTION_NAME(), p, mod, modid));
            continue;
        }
        if (!IS_HG_PORT(unit, p)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: %s, trunk port %d, is not HiGig Port\n"),
                       FUNCTION_NAME(), p));
            continue;
        }

        higig_port = p - SOC_PORT_MIN(unit, hg);
        if (higig_port >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: %s, trunk port %d not in valid range\n"),
                       FUNCTION_NAME(), p));
            continue;
        }

        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0 + higig_port]);
        if ((sp_info == NULL) || (sp_info->valid == FALSE)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_U(unit,
                                  "ERROR: %s, invalid fabric_group %d, unit %d\n"),
                       FUNCTION_NAME(), port, unit));
	    return(BCM_E_PARAM);
        }

        if (cosq >= sp_info->egroup[0].num_fifos) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid cos %d, noFifos %d unit %d\n"),
	               FUNCTION_NAME(),
	               port, sp_info->egroup[0].num_fifos, unit));
	    return(BCM_E_PARAM);
        }

        if (sp_info->egroup[0].num_fifos != (*nbr_fifos)) {
	    LOG_WARN(BSL_LS_BCM_PORT,
	             (BSL_META_U(unit,
	                         "ERROR: %s, NoFifos %d, does not match with the other entries NoFifos %d\n"),
	              FUNCTION_NAME(), sp_info->egroup[0].num_fifos, (*nbr_fifos)));
	    continue;
        }

        *(egress_groups + j) = sp_info->egroup[0].es_scheduler_level0_node;
        (*nbr_egress_groups) = j++;
    }

    return(rv);
}


int
bcm_sirius_port_size_set(int unit,
			 bcm_port_t fcd,
			 int cosq,
			 uint32  bytes_min,
			 uint32  bytes_max)
{
    int rv = BCM_E_NONE;
    int fifo_parm_index = 0;
    int is_full_asserted = FALSE, is_empty = 0;
 
    if (fcd >= SB_FAB_DEVICE_MAX_FABRIC_PORTS) {
        return(BCM_E_PARAM);
    }

    if ((cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
	(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid unicast cosq %d\n"),
                   FUNCTION_NAME(), cosq));
        return(BCM_E_PARAM);
    }

    fifo_parm_index = SOC_SBX_STATE(unit)->fcd_state[fcd].fifo + cosq;

    /* Assert Flow Control */
    rv = soc_sirius_fifo_force_full_set(unit, fifo_parm_index, TRUE);
    is_full_asserted = TRUE; /* This assumes this is the only place where Full is asserted */

    /* Check if fcd depth is zero i.e. empty */
    rv = soc_sirius_fifo_empty(unit, fifo_parm_index, &is_empty);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    if (is_empty == FALSE) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FIFO not empty fifo: %d, fcd: %d\n"),
                   fifo_parm_index, fcd));

        /* Flush port */
        rv = soc_sirius_port_flush(unit, fifo_parm_index);
        if (rv != BCM_E_NONE) {
            goto err;
        }
    }

    /* configure size settings */
    rv = soc_sirius_dt_mem_set(unit, fcd, cosq, bytes_min, bytes_max);

err:
    if (is_full_asserted == TRUE) {
	/* De-assert Flow Control */
        soc_sirius_fifo_force_full_set(unit, fifo_parm_index, FALSE);
    }
    return(rv);
}


int
bcm_sirius_port_size_get(int unit,
			 int fcd,
			 int cosq,
			 uint32  *bytes_min,
			 uint32  *bytes_max)
{
    int rv = BCM_E_NONE;

    if (fcd >= SB_FAB_DEVICE_MAX_FABRIC_PORTS) {
        return(BCM_E_PARAM);
    }

    if ((cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
	(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid unicast cosq %d\n"),
                   FUNCTION_NAME(), cosq));
        return(BCM_E_PARAM);
    }

    rv = soc_sirius_dt_mem_get(unit, fcd, cosq, bytes_min, bytes_max);
    return(rv);
}


int
bcm_sirius_port_multicast_size_set(int unit,
				   int fcd,
				   int cosq,
				   uint32  bytes_min,
				   uint32  bytes_max)
{
    int rv = BCM_E_NONE;
    bcm_port_t port = 0;
    int fifo_parm_index = 0;
    int is_mc_paused = FALSE, is_empty = 0;

    if ((fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS-1) || (fcd > SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid multicast fcd %d\n"),
                   FUNCTION_NAME(), fcd));
        return(BCM_E_PARAM);
    }

    if ((cosq != SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) && 
	(cosq != SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid multicast cosq %d\n"),
                   FUNCTION_NAME(), cosq));
        return(BCM_E_PARAM);
    }

    if (bytes_min < 128) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid bytes_min %d\n"),
                   FUNCTION_NAME(), bytes_min));
	return(BCM_E_PARAM);
    } 

    if (bytes_max < 128) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid bytes_max %d\n"),
                   FUNCTION_NAME(), bytes_max));
	return(BCM_E_PARAM);
    } 

    is_mc_paused = TRUE;

    for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS; port++) {

	fifo_parm_index = port * SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE + cosq;

	/* pause multicast flow control domain */
	rv = soc_sirius_fifo_force_full_set(unit, fifo_parm_index, TRUE);

	/* Check if fcd depth is zero i.e. empty */
	rv = soc_sirius_fifo_empty(unit, fifo_parm_index, &is_empty);

	if (rv != BCM_E_NONE) {
	    goto err;
	}
	
	if (is_empty == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_PORT,
	              (BSL_META_U(unit,
	                          "FIFO not empty fifo: %d, fcd: %d\n"),
	               fifo_parm_index, fcd));
	    
	    /* Flush port */
	    rv = soc_sirius_port_flush(unit, fifo_parm_index);
	    if (rv != BCM_E_NONE) {
		goto err;
	    }
	}
    }

    /* configure size settings */
    rv = soc_sirius_dt_mem_set(unit, fcd, cosq, bytes_min, bytes_max);

err:
    /* release multicast flow control domain */
    if (is_mc_paused) {
	for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS; port++) {

	    fifo_parm_index = port * SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE + cosq;

	    /* pause multicast flow control domain */
	    rv = soc_sirius_fifo_force_full_set(unit, fifo_parm_index, FALSE);
	}
    }

    return(rv);
}


int
bcm_sirius_port_multicast_size_get(int unit,
				   int fcd,
				   int cosq,
				   uint32  *bytes_min,
				   uint32  *bytes_max)
{
    int rv = BCM_E_NONE;

    if ((fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS-1) || (fcd > SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid multicast fcd %d\n"),
                   FUNCTION_NAME(), fcd));
        return(BCM_E_PARAM);
    }

    if ((cosq != SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) && 
	(cosq != SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
        LOG_ERROR(BSL_LS_BCM_PORT,
                  (BSL_META_U(unit,
                              "FUNCTION %s: Invalid multicast cosq %d\n"),
                   FUNCTION_NAME(), cosq));
        return(BCM_E_PARAM);
    }

    rv = soc_sirius_dt_mem_get(unit, fcd, cosq, bytes_min, bytes_max);
    return(rv);
}

#if defined(BCM_WARM_BOOT_SUPPORT)
int
_bcm_sirius_wb_port_state_sync(int unit, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    uint32 scache_len = 0;
    uint8  *ptr = NULL;
    uint32 byte_len, cid;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_PORT,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
                             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }
            
            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_PORT,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }
            
        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            break;
        default:
            break;
    }
    
    byte_len = sizeof(bcm_sbx_port_congestion_info_t) * 
               ((SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS + SB_FAB_DEVICE_SIRIUS_NUM_OOB_HCFC_PORTS) * 2);
    
    switch (operation) {
	case _WB_OP_SIZE:
	    scache_len += byte_len;
	    LOG_VERBOSE(BSL_LS_BCM_PORT,
	                (BSL_META_U(unit,
	                            "%s congestion_info total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            if (congestion_info[unit] != NULL) {
                sal_memcpy(congestion_info[unit], ptr, byte_len);
                ptr += byte_len;
            }
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_U(unit,
                                    "%s congestion_info total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    if (congestion_info[unit] != NULL) {
		sal_memcpy(ptr, congestion_info[unit], byte_len);
		ptr += byte_len;
	    }
	    LOG_VERBOSE(BSL_LS_BCM_PORT,
	                (BSL_META_U(unit,
	                            "%s congestion_info total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_DUMP:
	    LOG_VERBOSE(BSL_LS_BCM_PORT,
	                (BSL_META_U(unit,
	                            "%s congestion_info total %d bytes dumped on unit %d\n"),
	                 FUNCTION_NAME(), byte_len, unit));
            if (congestion_info[unit] != NULL) {
                for (cid = 0; cid < (2 * (SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS + SB_FAB_DEVICE_SIRIUS_NUM_OOB_HCFC_PORTS)); cid++) {
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "flags 0x%x, port_bits %d packets_per_sec %d,\n"),
                                 congestion_info[unit][cid].config.flags,
                                 congestion_info[unit][cid].config.port_bits,
                                 congestion_info[unit][cid].config.packets_per_sec));
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "src_port 0x%x, mcast_id %d traffic_class %d,\n"),
                                 congestion_info[unit][cid].config.src_port,
                                 congestion_info[unit][cid].config.multicast_id,
                                 congestion_info[unit][cid].config.traffic_class));
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "color %d, vlan %d pri %d cfi %d,\n"),
                                 congestion_info[unit][cid].config.color,
                                 congestion_info[unit][cid].config.vlan,
                                 congestion_info[unit][cid].config.pri ,
                                 congestion_info[unit][cid].config.cfi));
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "src_mac %2x:%2x:%2x:%2x:%2x:%2x, dest_mac %2x:%2x:%2x:%2x:%2x:%2x\n"),
                                 congestion_info[unit][cid].config.src_mac[0],
                                 congestion_info[unit][cid].config.src_mac[1],
                                 congestion_info[unit][cid].config.src_mac[2],
                                 congestion_info[unit][cid].config.src_mac[3],
                                 congestion_info[unit][cid].config.src_mac[4],
                                 congestion_info[unit][cid].config.src_mac[5],
                                 congestion_info[unit][cid].config.dest_mac[0],
                                 congestion_info[unit][cid].config.dest_mac[1],
                                 congestion_info[unit][cid].config.dest_mac[2],
                                 congestion_info[unit][cid].config.dest_mac[3],
                                 congestion_info[unit][cid].config.dest_mac[4],
                                 congestion_info[unit][cid].config.dest_mac[5]));
                    LOG_VERBOSE(BSL_LS_BCM_PORT,
                                (BSL_META_U(unit,
                                            "ethertype 0x%x opcode 0x%x\n"),
                                 congestion_info[unit][cid].config.ethertype,
                                 congestion_info[unit][cid].config.opcode));
                }
            }
	    break;
	default:
	    break;
    }    

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}
#endif
