/* 
 * $Id:$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        portctrl.c
 * Purpose:     SDK Port Control Layer
 *
 *              The purpose is to encapsulate port functionality
 *              related to the xxPORT block (i.e. XLPORT, CPORT)
 *              MAC and PHY.
 *
 *              Currently, only the PortMod library is being supported.
 *              The PortMod library provides support for the MAC, PHY,
 *              and xxPORT registers.
 *
 *              Callers of the Port Control routines should check
 *              before calling into them with the macro:
 *                  SOC_USE_PORTCTRL()
 *
 */

#include <shared/error.h>
#include <shared/bsl.h>
#include <sal/core/dpc.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/macutil.h>
#include <soc/drv.h>

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/portctrl.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/link.h>
#include <bcm_int/esw/xgs5.h>

#include <bcm/error.h>
#include <soc/firebolt.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/mbcm.h>

#if defined(BCM_TRIDENT2_SUPPORT)
#include <bcm_int/esw/trident2.h>
#include <soc/trident2.h>
#include <bcm_int/esw/trunk.h>
#endif /* BCM_TRIDENT2_SUPPORT */

#ifdef PORTMOD_SUPPORT
#include <soc/portmod/portmod.h>
#include <soc/portmod/portmod_common.h>
#include <soc/portmod/portmod_legacy_phy.h>
#include <soc/esw/portctrl_internal.h>

extern uint32_t phymod_dbg_mask;
extern uint32_t phymod_dbg_addr;
extern uint32_t phymod_dbg_lane;

/*
 * Define:
 *      PORTCTRL_PORT_RESOLVE
 * Purpose:
 *      Converts a GPORT or non-GPORT format port into:
 *      - BCM local port '_lport'
 *      - Port type used in PortMod functions '_pport'
 */
#define PORTCTRL_PORT_RESOLVE(_unit, _port, _lport, _pport)             \
    _bcm_esw_portctrl_port_resolve((_unit), (_port), (_lport), (_pport))

/*
 * Define:
 *      PORTCTRL_INIT_CHECK
 * Purpose:
 *      Checks that Port Control module has been initialized for given
 *      unit.  If failure, it causes routine to return with BCM_E_INIT.
 */
#define PORTCTRL_INIT_CHECK(_unit)    \
    SOC_IF_ERROR_RETURN(soc_esw_portctrl_init_check(_unit))


#define _ERR_MSG_MODULE_NAME           BSL_BCM_OTHER


#define BCM_ESW_PORTCTRL_PHY_REG_INDIRECT _SHR_PORT_PHY_REG_INDIRECT
#define BCM_ESW_PORTCTRL_PHYCTRL_PRBS_RX_ENABLE _SHR_PORT_PHY_CONTROL_PRBS_RX_ENABLE

STATIC int
_bcm_esw_portctrl_speed_chip_reconfigure(int unit, bcm_port_t port, int speed);

#endif /* PORTMOD_SUPPORT */

/*
 * Macros for use with encapsulation set related functions
 */
#define BCM_ESW_PORTCTRL_CFG_ENCAP_MODE 1
#define BCM_ESW_PORTCTRL_CFG_INTERFACE  2
#define BCM_ESW_PORTCTRL_CFG_SPEED      3

#define BCM_PA_ENCAP(e) \
    ((e == BCM_PORT_ENCAP_IEEE)                ? BCM_PA_ENCAP_IEEE                : \
     (e == BCM_PORT_ENCAP_HIGIG)               ? BCM_PA_ENCAP_HIGIG               : \
     (e == BCM_PORT_ENCAP_B5632)               ? BCM_PA_ENCAP_B5632               : \
     (e == BCM_PORT_ENCAP_HIGIG2)              ? BCM_PA_ENCAP_HIGIG2              : \
     (e == BCM_PORT_ENCAP_HIGIG2_LITE)         ? BCM_PA_ENCAP_HIGIG2_LITE         : \
     (e == BCM_PORT_ENCAP_HIGIG2_L2)           ? BCM_PA_ENCAP_HIGIG2_L2           : \
     (e == BCM_PORT_ENCAP_HIGIG2_IP_GRE)       ? BCM_PA_ENCAP_HIGIG2_IP_GRE       : \
     (e == BCM_PORT_ENCAP_SBX)                 ? BCM_PA_ENCAP_SBX                 : \
     (e == BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET) ? BCM_PA_ENCAP_HIGIG_OVER_ETHERNET : 0)

#ifdef PORTMOD_SUPPORT

/*
 * Function:
 *      _bcm_esw_portctrl_port_resolve
 * Purpose:
 *      Converts the given BCM port type, bcm_gport_t, to
 *      the BCM local port type and the port type expected by
 *      the PortMod functions.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      port     - (IN) BCM Port number, this can be in GPORT format.
 *      lport    - (OUT) Returns BCM logical local port number.
 *      pport    - (OUT) Returns port in expected PortMod port type.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_bcm_esw_portctrl_port_resolve(int unit, bcm_gport_t port,
                               bcm_port_t *lport, portctrl_pport_t *pport)
{
    /*
     * Currently, the PortMod library expects a logical local
     * port number, which is equivalent to the BCM port local port.
     */
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, lport));

    *pport = *lport;
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_from_portmod_ability
 * Purpose:
 *      Converts the _port_ability_t structure from PortMod to
 *      BCM _port_ability_t
 * Parameters:
 *      portmod_ability - (IN) PortMod _ability_ structure to convert.
 *      port_ability    - (OUT) Returns information in BCM bcm_port_ability_t.
 * Returns:
 *      None
 */
STATIC void
_bcm_esw_portctrl_from_portmod_ability(portmod_port_ability_t *portmod_ability,
                                       bcm_port_ability_t *port_ability)
{
    /*
     * Currently, the portmod_port_ability_t definition uses
     * _shr_port_ability_t, which is used by the 
     * SOC soc_port_ability_t and BCM bcm_port_ability_t.
     */
    *port_ability = *portmod_ability;

    return;
}


/*
 * Function:
 *      _bcm_esw_portctrl_port_fifo_reset
 * Purpose:
 *      Toggle the port soft reset.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */
STATIC int
_bcm_esw_portctrl_port_fifo_reset(int unit, bcm_port_t port,
                                  portctrl_pport_t pport)
{
    int phy_port, block, bindex, i;

    phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    for (i = 0; i < SOC_DRIVER(unit)->port_num_blktype; i++) {
        block = SOC_PORT_IDX_BLOCK(unit, phy_port, i);
        if ((SOC_BLOCK_INFO(unit, block).type == SOC_BLK_XLPORT) ||
            (SOC_BLOCK_INFO(unit, block).type == SOC_BLK_XLPORTB0) ||
            (SOC_BLOCK_INFO(unit, block).type == SOC_BLK_CLG2PORT) ||
            (SOC_BLOCK_INFO(unit, block).type == SOC_BLK_CLPORT)) {
            bindex = SOC_PORT_IDX_BINDEX(unit, phy_port, i);
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_soft_reset_toggle(unit, pport, bindex));
            break;
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_port_soft_reset
 * Purpose:
 *      Release or hold OBM, cell assembly and port logic from/in reset.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 *      reset   - (IN) O: Release from reset.
 *                     1: Hold port in reset.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */
STATIC int
_bcm_esw_portctrl_port_soft_reset(int unit, bcm_port_t port,
                                  portctrl_pport_t pport,
                                  int reset)
{
    int phy_port;
    int block;
    int bindex;
    int i;
    int flags = PORTMOD_PORT_REG_ACCESS_DEFAULT;

    phy_port = SOC_INFO(unit).port_l2p_mapping[port];

    for (i = 0; i < SOC_DRIVER(unit)->port_num_blktype; i++) {
        block = SOC_PORT_IDX_BLOCK(unit, phy_port, i);

        if ((SOC_BLOCK_INFO(unit, block).type == SOC_BLK_XLPORT) ||
            (SOC_BLOCK_INFO(unit, block).type == SOC_BLK_XLPORTB0) ||
            (SOC_BLOCK_INFO(unit, block).type == SOC_BLK_CLG2PORT) ||
            (SOC_BLOCK_INFO(unit, block).type == SOC_BLK_CLPORT)) {
            bindex = SOC_PORT_IDX_BINDEX(unit, phy_port, i);
            PORTMOD_IF_ERROR_RETURN
               (portmod_port_soft_reset_set(unit, pport, bindex, reset, flags));
            break;
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_tx_fifo_empty
 * Purpose:
 *      Poll until the MAC TX FIFO is empty, or until
 *      it times out.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */
STATIC int
_bcm_esw_portctrl_tx_fifo_empty(int unit, bcm_port_t port,
                                portctrl_pport_t pport)
{
    uint32 cell_count;
    soc_timeout_t to;
    int wait_time = 250000;

    if (SAL_BOOT_QUICKTURN) {
        wait_time *= 20;
    }

    /* Wait until MAC TX fifo cell count is 0 */
    soc_timeout_init(&to, wait_time, 0);
    for (;;) {
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_txfifo_cell_cnt_get(unit, pport, &cell_count));

        if (cell_count == 0) {
            break;
        }
        if (soc_timeout_check(&to)) {
            LOG_ERROR(BSL_LS_BCM_PORT,
                      (BSL_META_UP(unit, port,
                                   "ERROR: u=%d p=%d timeout draining "
                                   "MAC TX FIFO (%d cells remain)\n"),
                       unit, port, cell_count));
            return BCM_E_INTERNAL;
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_drain_cells
 * Purpose:
 *      Drain cells for given port.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */
STATIC int
_bcm_esw_portctrl_drain_cells(int unit, bcm_port_t port,
                              portctrl_pport_t pport)
{
    portmod_drain_cells_t drain_cells;
    uint32 cell_count;
    soc_timeout_t to;
    int rv = BCM_E_NONE;
    int wait_time = 250000;

    if (SAL_BOOT_QUICKTURN) {
        wait_time *= 20;
    }

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_drain_cell_get(unit, pport, &drain_cells));

    /* Start TX FIFO draining */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_drain_cell_start(unit, pport));

    /* Reset EP credit before de-assert SOFT_RESET */
    SOC_IF_ERROR_RETURN(soc_port_credit_reset(unit, port));

    /* De-assert SOFT_RESET to let the drain start */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_mac_reset_set(unit, pport, 0));

    /* Wait until mmu cell count is 0 */
    rv = soc_egress_drain_cells(unit, port, wait_time);
    if (BCM_E_NONE == rv) {
        /* Wait until TX fifo cell count is 0 */
        soc_timeout_init(&to, wait_time, 0);
        for (;;) {
            rv = portmod_port_txfifo_cell_cnt_get(unit, pport, &cell_count);
            if (BCM_E_NONE != rv) {
                break;
            }
            if (cell_count == 0) {
                break;
            }
            if (soc_timeout_check(&to)) {
                LOG_ERROR(BSL_LS_BCM_PORT,
                          (BSL_META_UP(unit, port,
                                       "ERROR: u=%d p=%d timeout draining "
                                       "TX FIFO (%d cells remain)\n"),
                                       unit, port, cell_count));
                rv = BCM_E_INTERNAL;
                break;
            }
        }
    }
   
    /* Stop TX FIFO draining */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_drain_cell_stop(unit, pport, &drain_cells));

    return rv;
}


/*
 * Function:
 *      _bcm_esw_portctrl_mmu_flush
 * Purpose:
 *      Flush port in MMU.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK hed.
 */
STATIC int
_bcm_esw_portctrl_mmu_flush(int unit, bcm_port_t port,
                            portctrl_pport_t pport)
{
    portmod_pfc_control_t pfc, prev_pfc;
    portmod_llfc_control_t llfc, prev_llfc;
    portmod_pause_control_t pause, prev_pause;
    int rv = BCM_E_NONE;
    int wait_time = 250000;

    if (SAL_BOOT_QUICKTURN) {
        wait_time *= 20;
    }


    /* Disable flow control (store current settings) */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pause_control_get(unit, pport, &pause));
    prev_pause = pause;
    pause.rx_enable = 0;
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pause_control_set(unit, pport, &pause));

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_get(unit, pport, &pfc));
    prev_pfc = pfc;
    pfc.rx_enable = 0;
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_set(unit, pport, &pfc));

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_llfc_control_get(unit, pport, &llfc));
    prev_llfc = llfc;
    llfc.rx_enable = 0;
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_llfc_control_set(unit, pport, &llfc));

    /* Wait until mmu cell count is 0 */
    rv = soc_egress_drain_cells(unit, port, 250000);

    /* Restore flow control settings */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pause_control_set(unit, pport, &prev_pause));
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_set(unit, pport, &prev_pfc));
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_llfc_control_set(unit, pport, &prev_llfc));

    /*
     * Delay 1 usec to ensure all cells drained from MMU
     * during the flush have left EP.
     */
    sal_udelay(1);

    return rv;
}


/*
 * Function:
 *      _bcm_esw_portctrl_enable_set
 * Purpose:
 *      Core function to enable or disable a port.
 *      This includes all the necessary steps to correctly bring
 *      up or down a port.
 *
 *      The sequence to bring a port UP or DOWN requires a series of
 *      interleaved steps that falls inside and outside the PortMod library.
 *      Some operations need to occur in the xxPORT, or MAC, or PHY blocks,
 *      whereas others need to occur at the EP, or MMU blocks.
 *
 *      There are two approaches:
 *      1) SDK drives the logic calling the corresponding PortMod and
 *         non-PortMod routines.
 *      2) PortMod drive the logic calling callback routines provided
 *         by SDK to perform operations outside the PortMod.
 *
 *      At the moment (1) is implemented.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 *      flags   - (IN) Indicates whether to enable PHY, MAC, or both.
 *      enable  - (IN) Indicates whether to enable or disable port.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 *      This follows the new sequence provided by the design team.
 *
 *      Currently, the PortMod API portmod_port_enable_set() do enable/disable
 *      the MAC does not have the complete bringup sequence.  Callers
 *      should be calling this routine instead to bring the port up or down
 *      properly.
 */
STATIC int
_bcm_esw_portctrl_enable_set(int unit, bcm_port_t port,
                             portctrl_pport_t pport,
                             int flags, int enable)
{
    pbmp_t mask;
    int flags_temp;
    int mac_reset;
    int wait_us, mngport;

    if (enable) {
        /*
         * PORT UP
         */

        /* Enable PHY if PHY flag is set */
        if (PORTMOD_PORT_ENABLE_PHY_GET(flags)) {
            flags_temp = flags;
            PORTMOD_PORT_ENABLE_MAC_CLR(flags_temp);
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_enable_set(unit, pport, flags_temp, 1));
        }

        /*
         * Check if MAC needs to be modified based on whether
         * we want to enable or disable the MAC.
         * If not changes are needed, assume port is
         * already UP and just skip enabling MAC.
         */
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_mac_reset_check(unit, pport,
                                          enable, &mac_reset));
        /*
         * If MAC flag is set and MAC is not UP yet, continue with rest of
         * sequence to bring the port UP, including resetting credits,
         * cells draining, etc.
         */
        if (mac_reset && PORTMOD_PORT_ENABLE_MAC_GET(flags)) {

            if (soc_feature(unit, soc_feature_port_leverage)) {
                if (BCM_ESW_PORT_DRV(unit)!= NULL) {
                    if (BCM_ESW_PORT_DRV(unit)->port_enable_set != NULL) {
                        BCM_IF_ERROR_RETURN
                            (BCM_ESW_PORT_DRV(unit)->port_enable_set(unit,
                                                                     port,
                                                                     1));
                    }
                }
            } else {
                /*
                 * Initialize EP credits,
                 * release EDB port buffer reset and enable cell request
                 * generation in EP.
                 */
                SOC_IF_ERROR_RETURN
                    (soc_port_egress_buffer_sft_reset(unit, port, 0));

                /* Release OBM, cell assembly and port logic from reset */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_portctrl_port_soft_reset(unit, port, pport, 0));

                /* Release Ingress buffers from reset */
                SOC_IF_ERROR_RETURN
                    (soc_port_ingress_buffer_reset(unit, port, 0));

                /* Enable MAC TX and RX and take MAC out of reset */
                PORTMOD_IF_ERROR_RETURN
                    (portmod_port_drain_cells_rx_enable(unit, pport, 1));
            }
        }

        /* Add port to EPC_LINK */
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_ADD(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));

        /* Enable forwarding traffic */
        if (soc_feature(unit, soc_feature_ingress_dest_port_enable)) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_ingress_dest_enable(unit, port, 1));
        }
    } else {
        /*
         * PORT DOWN
         */

        /* Disable all forwarding traffic */
        if (soc_feature(unit, soc_feature_ingress_dest_port_enable)) {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_ingress_dest_enable(unit, port, 0));
        }

        /*
         * Remove from EPC_LINK to prevent packet queued
         * to this port from TX
         */
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_REMOVE(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));

        /* Wait 8ms if management port configured to 10/100, else 80 us.
         * please refer TH2/TD2p_uA_flexport.pdf, Section 6.6.1 */
        wait_us = 80;
        SOC_PBMP_ITER(PBMP_MANAGEMENT(unit), mngport) {
            if (SOC_INFO(unit).port_init_speed[mngport] <= 100) {
                wait_us = 8000;
                break;
            }
        }
        if (SAL_BOOT_QUICKTURN) {
            wait_us *= 10000;
        }
        sal_usleep(wait_us);

        /*
         * Due to a HW issue, the MAC RX needs to be disabled
         * before disabling PHY (SW workaround)
         */

        /*
         * If only PHY flag is set, just disable PHY.
         * Otherwise, PHY is disabled later along with the MAC
         * following the right order.
         */
        if (PORTMOD_PORT_ENABLE_PHY_GET(flags) &&
            !PORTMOD_PORT_ENABLE_MAC_GET(flags)) {
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_enable_set(unit, pport, flags, 0));
        }

        /*
         * If MAC flag is set, continue with rest of sequence
         * to bring the port DOWN.
         */
        if (PORTMOD_PORT_ENABLE_MAC_GET(flags)) {

            /*
             * Disable MAC RX and PHY (if flag is supplied).
             *
             * NOTE: MAC RX needs to be disabled before the TSC.
             */
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_rx_mac_enable_set(unit, pport, 0));
            if (PORTMOD_PORT_ENABLE_PHY_GET(flags)) {
                PORTMOD_IF_ERROR_RETURN
                    (portmod_port_enable_set(unit, pport,
                                             PORTMOD_PORT_ENABLE_PHY, 0));
            }

            /*
             * Check if MAC needs to be modified based on whether
             * we want to enable or disable the MAC.
             * If not changes are needed, assume port is
             * already DOWN and just return.
             */
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_mac_reset_check(unit, pport,
                                              enable, &mac_reset));
            if (!mac_reset) {
                return BCM_E_NONE;
            }

            if (soc_feature(unit, soc_feature_port_leverage)) {
                if (BCM_ESW_PORT_DRV(unit)!= NULL) {
                    if (BCM_ESW_PORT_DRV(unit)->port_enable_set != NULL) {
                        BCM_IF_ERROR_RETURN
                            (BCM_ESW_PORT_DRV(unit)->port_enable_set(unit,
                                                                     port,
                                                                     0));
                    }
                }
            } else {

                /* Put port in flush state using MAC discard */
                PORTMOD_IF_ERROR_RETURN
                    (portmod_port_discard_set(unit, pport, 1));

                /* Reset Ingress buffers */
                SOC_IF_ERROR_RETURN
                    (soc_port_ingress_buffer_reset(unit, port, 1));

                /* Hold OBM, cell assembly and port logic in reset */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_portctrl_port_soft_reset(unit, port, pport, 1));

                /* MMU flush */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_portctrl_mmu_flush(unit, port, pport));

                /* Reset Egress buffers */
                SOC_IF_ERROR_RETURN
                    (soc_port_egress_buffer_sft_reset(unit, port, 1));

                /* Poll until MAC TX FIFO is empty */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_portctrl_tx_fifo_empty(unit, port, pport));

                /* Disable MAC TX and power down TSC */
                PORTMOD_IF_ERROR_RETURN
                    (portmod_port_tx_down(unit, pport));
            }
        } /* MAC flag is set */

    } /* PORT DOWN */

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_egress_queue_drain
 * Purpose:
 *      Drain the egress queues without bringing down the port.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 *      Helper to bcm_esw_portctrl_egress_queue_drain.
 */
STATIC int
_bcm_esw_portctrl_egress_queue_drain(int unit, bcm_port_t port,
                                     portctrl_pport_t pport)
{
    uint64 mac_ctrl;
    int rx_enable = 0;
    pbmp_t mask;
    int is_active = 0;

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_egress_queue_drain_get(unit, pport,
                                             &mac_ctrl, &rx_enable));

    /* Remove port from EPC_LINK */
    soc_link_mask2_get(unit, &mask);
    if (SOC_PBMP_MEMBER(mask, port)) {
        is_active = 1;
        SOC_PBMP_PORT_REMOVE(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
    }

    /* Drain cells */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_portctrl_drain_cells(unit, port, pport));

    /* Reset port FIFO */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_portctrl_port_fifo_reset(unit, port, pport));

    /* Put port into SOFT_RESET */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_mac_reset_set(unit, pport, 1));

    /* Reset EP credit before de-assert SOFT_RESET */
    SOC_IF_ERROR_RETURN(soc_port_credit_reset(unit, port));

    /* Enable TX, set RX, de-assert SOFT_RESET */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_egress_queue_drain_rx_en(unit, pport, rx_enable));

    /* Restore XLMAC_CTRL to original value */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_mac_ctrl_set(unit, pport, mac_ctrl));

    /* Add port to EPC_LINK */
    if(is_active) {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_ADD(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
    }
 
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_portctrl_interface_config_set
 * Purpose:
 *      Core function to set the port interface configuration.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      port    - (IN) Logical BCM Port number (non-GPORT).
 *      pport   - (IN) Port type expected by PortMod.
 *      if_cfg  - (IN) Port Configuration to set.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 *
 *      SDK drivers should NOT call portmod_port_interface_config_set()
 *      directly, expect for this routine.  Drivers should call
 *      this routine instead to set the port interface configuration.
 *
 *      In order to change a port inteface configuration, the port must
 *      be properly be disabled and then re-enabled back.  Since the
 *      port bringup sequence is currenlty in the SDK and not in the PortMod
 *      library, the complete function to modify the port configuration will
 *      resides in the SDK driver.
 */
STATIC int
_bcm_esw_portctrl_interface_config_set(int unit, bcm_port_t port,
                                       portctrl_pport_t pport,
                                       portmod_port_interface_config_t *if_cfg,
                                       uint32 all_phy)
{
    int enable, phy_init_flag = PORTMOD_INIT_F_EXTERNAL_MOST_ONLY;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_enable_get(unit, port, PORTMOD_PORT_ENABLE_MAC, &enable));

    if (enable) {
        /* Disable port */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                          PORTMOD_PORT_ENABLE_MAC, 0));
    }

    if (if_cfg->encap_mode & BCM_PORT_ENCAP_HIGIG2) {
        if_cfg->interface_modes |= PHYMOD_INTF_MODES_HIGIG;
    } else {
        if_cfg->interface_modes &= ~PHYMOD_INTF_MODES_HIGIG;
    }

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_interface_config_set(unit, pport, if_cfg, phy_init_flag));

    if (enable) {
        /* Re-enable port */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                          PORTMOD_PORT_ENABLE_MAC, 1));
    }

    return BCM_E_NONE;
}
#endif /* PORTMOD_SUPPORT */

/*
 * Function:
 *      bcmi_esw_portctrl_probe
 * Purpose:
 *      Probe given port and add to Portmod.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) Port number.
 *      okay      - (OUT) Returns true is port was successfully added.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *     If port is already attached/added in the PortMod, port
 *     is first removed(detached) and then added again.
 */
int
bcmi_esw_portctrl_probe(int unit, bcm_gport_t port, int init_flag, int *okay)
{
#ifdef PORTMOD_SUPPORT
    portmod_pbmp_t p_pbmp;
    int rv = BCM_E_NONE, valid, flags=0, lane=0, num_lanes, speed;
    portctrl_pport_t pport;
    bcm_port_if_t intf;

    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    *okay = FALSE;

    /* Check if port is already in PM */
    rv = portmod_port_is_valid(unit, pport, &valid);
    if (PORTMOD_FAILURE(rv)) {
        return rv;
    }


    /* Remove port from PM if this has been already added */
    if (valid) {
        if ((!init_flag) || (init_flag == PORTMOD_PORT_ADD_F_INIT_CORE_PROBE)) {
            rv = bcm_esw_port_interface_get(unit, port, &intf);
            /* Don't update speed if the port is attached to PHY_NULL */
            if (BCM_SUCCESS(rv) && (intf != SOC_PORT_IF_NULL)) {
                speed = SOC_INFO(unit).port_init_speed[port] > 0 ?
                        SOC_INFO(unit).port_init_speed[port] :
                        SOC_INFO(unit).port_speed_max[port];
                rv = bcmi_esw_portctrl_speed_set(unit, port, speed);
            }
            if (BCM_SUCCESS(rv)) {
                PORT_LOCK(unit);
                /* PortMod requires port to be disabled before removing it */
                PORTMOD_PORT_ENABLE_PHY_SET(flags);
                PORTMOD_PORT_ENABLE_MAC_SET(flags);
                rv = _bcm_esw_portctrl_enable_set(unit, port, pport, flags, 0);
                if (BCM_SUCCESS(rv)) {
                    rv = soc_esw_portctrl_delete(unit, pport);
                }
                PORT_UNLOCK(unit);
            }
        }
    }

    /* Add port to PM */
    if (SOC_SUCCESS(rv)) {
        if ((!init_flag) || (init_flag == PORTMOD_PORT_ADD_F_INIT_CORE_PROBE)) {
            PORTMOD_PBMP_CLEAR(p_pbmp);
            num_lanes = SOC_INFO(unit).port_num_lanes[port];
            if(num_lanes == 10) {
                num_lanes=12;
            }
            for (lane = 0; lane < num_lanes; lane++) {
                PORT_LOCK(unit);
                portmod_xphy_lane_detach(unit, SOC_INFO(unit).port_l2p_mapping[port]+lane, 1); 
                PORT_UNLOCK(unit);
                PORTMOD_PBMP_PORT_ADD(p_pbmp, SOC_INFO(unit).port_l2p_mapping[port] + lane);
            }
            PORT_LOCK(unit);
            rv = soc_esw_portctrl_setup_ext_phy_add(unit, p_pbmp);
            PORT_UNLOCK(unit);
            if (SOC_FAILURE(rv)) {
                return rv;
            }
        }
        PORT_LOCK(unit);
        rv = soc_esw_portctrl_add(unit, port, init_flag);
        PORT_UNLOCK(unit);
    }

    if (SOC_SUCCESS(rv)) {
        if ((init_flag == PORTMOD_PORT_ADD_F_INIT_PASS2) || !init_flag) {
            /* Probe function should leave port disabled */
            PORT_LOCK(unit);
            rv = _bcm_esw_portctrl_enable_set(unit, port, pport,
                                   PORTMOD_PORT_ENABLE_PHY, 0);
            PORT_UNLOCK(unit);
            if (SOC_FAILURE(rv)) {
                return rv;
            }
        }
    }

    *okay = TRUE;

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_probe_pbmp
 * Purpose:
 *      Probe give port bitmap to identify which ports to add to Portmod.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      pbmp      - (IN) Port bitmap.
 *      okay_pbmp - (OUT) Port bitmap okay.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_probe_pbmp(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
#ifdef PORTMOD_SUPPORT
    pbmp_t     lcl_pbmp;
    soc_port_t port;
    int okay = 0;
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    phymod_operation_mode_t phy_op_mode;
    _bcm_port_info_t *port_info = NULL;

    PORTCTRL_INIT_CHECK(unit);

    SOC_PBMP_CLEAR(lcl_pbmp);
    SOC_PBMP_CLEAR(*okay_pbmp);

    PBMP_ITER(pbmp, port) {
        /* Skip RCPU ports */
#ifdef BCM_RCPU_SUPPORT
        if (SOC_IS_RCPU_ONLY(unit) &&
            SOC_PORT_VALID(unit, RCPU_PORT(unit)) &&
            BCM_PBMP_MEMBER(pbmp, RCPU_PORT(unit))) {
            continue;
        }
#endif /* BCM_RCPU_SUPPORT */
        if (IS_TDM_PORT(unit, port)) {
            continue;
        }
        SOC_PBMP_PORT_ADD(lcl_pbmp, port);
    }

    if (!SOC_WARM_BOOT(unit)) {

        /*step1: probe Serdes and external PHY core*/
        PBMP_ITER(lcl_pbmp, port) {
            BCM_IF_ERROR_RETURN
                (bcmi_esw_portctrl_probe(unit, port, 
                        PORTMOD_PORT_ADD_F_INIT_CORE_PROBE, &okay));
        }

        /*step2 : initialize PASS1 for SerDes and external PHY*/
        PBMP_ITER(lcl_pbmp, port) {
            BCM_IF_ERROR_RETURN
                (bcmi_esw_portctrl_probe(unit, port, 
                        PORTMOD_PORT_ADD_F_INIT_PASS1, &okay));
        }

        /* step3:broadcast firmware download for all external phys inculde legacy and Phymod PHYs*/
        BCM_IF_ERROR_RETURN(portmod_legacy_ext_phy_init(unit, pbmp));
        BCM_IF_ERROR_RETURN(portmod_common_ext_phy_fw_bcst(unit, lcl_pbmp));

        /*step4:initialize PASS2 for Serdes and external PHY*/
        PBMP_ITER(lcl_pbmp, port) {
            BCM_IF_ERROR_RETURN
                (bcmi_esw_portctrl_probe(unit, port, 
                      PORTMOD_PORT_ADD_F_INIT_PASS2, &okay));
            if (okay) {
                SOC_PBMP_PORT_ADD(*okay_pbmp, port);
                soc_counter_port_pbmp_add(unit, port);
            }
        }
    }

    /* warmboot boot, restore external phys data base. */
    if (SOC_WARM_BOOT(unit)) {
        BCM_IF_ERROR_RETURN(soc_esw_portctrl_xphy_wb_db_restore(unit));
    }

    PBMP_ITER(pbmp, port) {
        BCM_IF_ERROR_RETURN
            (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));
        if (!SOC_WARM_BOOT(unit)) {
            rv = _bcm_esw_portctrl_enable_set(unit, port, pport,
                                              PORTMOD_PORT_ENABLE_MAC, FALSE);
            if (BCM_FAILURE(rv)) {
                SOC_PBMP_PORT_REMOVE(*okay_pbmp, port);
                soc_counter_port_pbmp_remove(unit, port);
                LOG_WARN(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "MAC init failed on port %s\n"),
                          SOC_PORT_NAME(unit, port)));
                break;
            }
        } else {
            portmod_port_interface_config_t interface_config;
            portmod_port_init_config_t      init_config;
            portmod_pbmp_t p_pbmp;
            int lane=0, num_lanes=0;
            int legacy_phy = 0;

            rv = soc_esw_portctrl_config_get(unit, port,
                                     &interface_config,
                                     &init_config,
                                     &phy_op_mode);
            if (BCM_FAILURE(rv)) {
                break;
            }

            /* restore external phy attach */
            PORTMOD_PBMP_CLEAR(p_pbmp);
            num_lanes = SOC_INFO(unit).port_num_lanes[port];
            if(num_lanes == 10) num_lanes=12;
            for (lane = 0; lane < num_lanes; lane++) {
                PORTMOD_PBMP_PORT_ADD(p_pbmp, SOC_INFO(unit).port_l2p_mapping[port] + lane);
            }
            SOC_IF_ERROR_RETURN(soc_esw_portctrl_setup_ext_phy_add(unit, p_pbmp));

            /* Restore ports with legacy external phys */
            rv = portmod_port_check_legacy_phy(unit, port, &legacy_phy);
            if (BCM_FAILURE(rv)) {
                break;
            }

            if (legacy_phy) {
                BCM_IF_ERROR_RETURN(portmod_port_legacy_phy_init(unit, port));
            }

            rv = portmod_port_warmboot_db_restore(unit, pport, &interface_config, &init_config, phy_op_mode);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }

        if (soc_property_get(unit, spn_SAME_SPEED_INTF_DO_NOT_OVERWRITE,
                             (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM))) {
            _bcm_port_info_access(unit, port, &port_info);
            if ((rv = bcm_esw_port_interface_get(unit, port, 
                                                 &port_info->intf)) < 0) {
                port_info->intf = BCM_PORT_IF_NOCXN;
            }
        }

        if (soc_property_port_get(unit, port, spn_FCMAP_ENABLE, 0)) {
            soc_persist_t       *sop = SOC_PERSIST(unit);
            SOC_PBMP_PORT_ADD(sop->lc_pbm_fc, port);
        }
    }

    return BCM_E_NONE;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_ability_get
 * Purpose:
 *      Retrieve the local port abilities.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Port number.
 *      port_ability - (OUT) Returns bcm_port_ability_t structure information.
 *      ability_mask - (OUT) If !NULL, returns mask of BCM_PORT_ABIL_
 *                           values indicating the ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_ability_get(int unit, bcm_gport_t port,
                              bcm_port_ability_t *port_ability,
                              bcm_port_abil_t *ability_mask)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    portmod_port_ability_t portmod_ability;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if(!(SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port))) {
       return BCM_E_PORT;
    }

    sal_memset(port_ability, 0, sizeof(*port_ability));
    sal_memset(&portmod_ability, 0, sizeof(portmod_ability));
    if (ability_mask != NULL) {
        *ability_mask = 0;
    }

    PORT_LOCK(unit);
    rv = portmod_port_ability_local_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &portmod_ability);
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        _bcm_esw_portctrl_from_portmod_ability(&portmod_ability,
                                               port_ability);

#ifdef BCM_TOMAHAWK2_SUPPORT
        if (soc_feature(unit, soc_feature_flexport_based_speed_set)) {
            bcm_port_abil_t mask;

            BCM_IF_ERROR_RETURN
                (bcmi_xgs5_flexport_based_speed_ability_get(unit, port, &mask));
            port_ability->speed_full_duplex &= mask;
            port_ability->speed_half_duplex &= mask;
        }
#endif
        port_ability->loopback  |= BCM_PORT_ABILITY_LB_NONE;
        if(SAL_BOOT_SIMULATION) {
            port_ability->loopback |= BCM_PORT_ABILITY_LB_MAC;
            port_ability->loopback &= ~BCM_PORT_ABILITY_LB_PHY;
        }

        if ((soc_feature(unit, soc_feature_embedded_higig)) &&
            IS_E_PORT(unit, port)) {
            port_ability->encap |= BCM_PA_ENCAP_HIGIG2_L2;
            port_ability->encap |= BCM_PA_ENCAP_HIGIG2_IP_GRE;
        }

        if ((soc_feature(unit, soc_feature_higig_over_ethernet))) {
            port_ability->encap |= BCM_PA_ENCAP_HIGIG_OVER_ETHERNET;
        }

        if (soc_feature(unit, soc_feature_no_higig_plus)) {
            port_ability->encap &= ~BCM_PA_ENCAP_HIGIG;
        }

        SOC_IF_ERROR_RETURN(
            soc_esw_portctrl_port_ability_update(unit, port, port_ability));

        /* Convert to BCM_PORT_ABIL_ mask if needed */
        if (ability_mask != NULL) {
            rv = soc_port_ability_to_mode(port_ability, ability_mask);
        }
    }

    if (ability_mask != NULL) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_UP(unit, port,
                              "Port ability get: u=%d p=%d abil=0x%x "
                              "rv=%d\n"),
                  unit, port, *ability_mask, rv));
    } else {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_UP(unit, port,
                              "Port ability get: u=%d p=%d rv=%d\n"),
                  unit, port, rv));
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_UP(unit, port,
                             "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x abl_get\n"
                             "Interface=0x%08x Medium=0x%08x EEE=0x%08x "
                             "Loopback=0x%08x Flags=0x%08x\n"),
                 port_ability->speed_half_duplex,
                 port_ability->speed_full_duplex,
                 port_ability->pause, port_ability->interface,
                 port_ability->medium, port_ability->eee,
                 port_ability->loopback, port_ability->flags));

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_ability_remote_get
 * Purpose:
 *      Retrieve the remote advertised port abilities.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Port number.
 *      port_ability - (OUT) Returns bcm_port_ability_t structure information.
 *      ability_mask - (OUT) If !NULL, returns mask of BCM_PORT_ABIL_
 *                           values indicating the ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_ability_remote_get(int unit, bcm_gport_t port,
                                     bcm_port_ability_t *port_ability,
                                     bcm_port_abil_t *ability_mask)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    portmod_port_ability_t portmod_ability;
    phymod_autoneg_status_t an_status;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    sal_memset(port_ability, 0, sizeof(*port_ability));
    sal_memset(&portmod_ability, 0, sizeof(portmod_ability));
    if (ability_mask != NULL) {
        *ability_mask = 0;
    }

    PORT_LOCK(unit);

    rv = portmod_port_autoneg_status_get(unit, pport, &an_status);

    if (PORTMOD_SUCCESS(rv)) {
        if (an_status.enabled && an_status.locked) {
            rv = portmod_port_ability_remote_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY,
                                                 &portmod_ability);
        }
    }

    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        _bcm_esw_portctrl_from_portmod_ability(&portmod_ability,
                                               port_ability);

        /* Convert to BCM_PORT_ABIL_ mask if needed */
        if (ability_mask != NULL) {
            rv = soc_port_ability_to_mode(port_ability, ability_mask);
        }
    }

    if (ability_mask != NULL) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_UP(unit, port,
                              "Port ability remote get: u=%d p=%d abil=0x%x "
                              "rv=%d\n"),
                  unit, port, *ability_mask, rv));
    } else {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_UP(unit, port,
                              "Port ability remote get: u=%d p=%d rv=%d\n"),
                  unit, port, rv));
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_UP(unit, port,
                             "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x abl_remote_get\n"
                             "Interface=0x%08x Medium=0x%08x "
                             "Loopback=0x%08x Flags=0x%08x\n"),
                 port_ability->speed_half_duplex,
                 port_ability->speed_full_duplex,
                 port_ability->pause, port_ability->interface,
                 port_ability->medium,
                 port_ability->loopback, port_ability->flags));

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_enable_get
 * Purpose:
 *      Gets the enable state as defined by bcm_port_enable_set()
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      enable - (OUT) TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The PHY enable holds the port enable state set by the user.
 *      The MAC enable transitions up and down automatically via linkscan
 *      even if user port enable is always up.
 */
int
bcmi_esw_portctrl_enable_get(int unit, bcm_gport_t port, int *enable)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    _bcm_port_info_t *port_info = NULL;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), port)) {
        *enable = 0;
        return BCM_E_NONE;
    }

    PORT_LOCK(unit);

    rv = portmod_port_enable_get(unit, pport, PORTMOD_PORT_ENABLE_PHY, enable);

    _bcm_port_info_access(unit, port, &port_info);

    if (port_info == NULL) {
        PORT_UNLOCK(unit);
        return BCM_E_INIT;
    }
    *enable = port_info->enable & (*enable);

    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Port enable get: u=%d p=%d rv=%d enable=%d\n"),
              unit, port, rv, *enable));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_enable_set
 * Purpose:
 *      Physically enable/disable the MAC/PHY on this port.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      enable - (IN) TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If linkscan is running, it also controls the MAC enable state.
 */
int
bcmi_esw_portctrl_enable_set(int unit, bcm_gport_t port, int enable)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int link, loopback = BCM_PORT_LOOPBACK_NONE;
    soc_persist_t *sop = SOC_PERSIST(unit);
    _bcm_port_info_t *port_info;
    int flags = 0;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

#ifdef BCM_RCPU_SUPPORT
    if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
#endif /* BCM_RCPU_SUPPORT */

    BCM_IF_ERROR_RETURN
        (bcmi_esw_portctrl_loopback_get(unit, port, &loopback));

    PORT_LOCK(unit);

    if (enable) {
        if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, all), port)) {
            PORT_UNLOCK(unit);
            return BCM_E_NONE;
        }

        rv = _bcm_esw_portctrl_enable_set(unit, port, pport,
                                          PORTMOD_PORT_ENABLE_PHY, 1);
        if (BCM_FAILURE(rv)) {
            PORT_UNLOCK(unit);
            return rv;
        }

        /* Get link status after PHY state has been set */
        rv = bcm_esw_port_link_status_get(unit, port, &link);
        if (BCM_FAILURE(rv)) {
            if (rv == BCM_E_INIT) {
                link = FALSE;
                rv = BCM_E_NONE;
            } else {
                PORT_UNLOCK(unit);
                return rv;
            }
        }

        if (link || (loopback != BCM_PORT_LOOPBACK_NONE)
            || SOC_PBMP_MEMBER(sop->lc_pbm_fc, port)
            || SOC_PBMP_MEMBER(sop->lc_pbm_linkdown_tx, port)) {
            rv = _bcm_esw_portctrl_enable_set(unit, port, pport,
                                              PORTMOD_PORT_ENABLE_MAC, 1);
            if (BCM_FAILURE(rv)) {
                PORT_UNLOCK(unit);
                return rv;
            }
        }

    } else {

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_UP(unit, port,
                                 "Disable and isolate u=%d p=%d\n"),
                     unit, port));

        /*
         * When the port is configured to MAC loopback,
         * cannot disable MAC.
         */
        PORTMOD_PORT_ENABLE_PHY_SET(flags);
        if (loopback != BCM_PORT_LOOPBACK_MAC) {
            PORTMOD_PORT_ENABLE_MAC_SET(flags);
        }
        rv = _bcm_esw_portctrl_enable_set(unit, port, pport, flags, 0);
        if (BCM_FAILURE(rv)) {
            PORT_UNLOCK(unit);
            return rv;
        }
    }

    _bcm_port_info_access(unit, port, &port_info);
    port_info->enable = enable;

    PORT_UNLOCK(unit);

    /* Unlock before link call */
    if (loopback != BCM_PORT_LOOPBACK_NONE) {
        if (loopback == BCM_PORT_LOOPBACK_MAC) {
            rv = _bcm_esw_link_force(unit, 0, port, TRUE, TRUE);
        } else {
            rv = _bcm_esw_link_force(unit, 0, port, TRUE, enable);
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Port enable set: u=%d p=%d enable=%d rv=%d\n"),
              unit, port, enable, rv));

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_phy_enable_set
 * Purpose:
 *      Physically enable/disable the PHY on this port.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      enable - (IN) TRUE, PHY port is enabled, FALSE PHY port is disabled.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Unlike bcmi_esw_portctl_enable_set(), this routine
 *      only enables the PHY.  It does not contain any logic
 *      to handle loopback cases, MAC, or current port state.
 */
int
bcmi_esw_portctrl_phy_enable_set(int unit, bcm_gport_t port, int enable)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_enable_set(unit, pport,
                                 PORTMOD_PORT_ENABLE_PHY, enable);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_egress_queue_drain
 * Purpose:
 *      Drain the egress queues without bringing down the port.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_portctrl_egress_queue_drain(int unit, bcm_gport_t port)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = _bcm_esw_portctrl_egress_queue_drain(unit, port, pport);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_notify
 * Description:
 *      Notify application the chip reconfigure failure from a auto-neg speed.
 * Parameters:
 *      unit_vp   - (IN) Device number.
 *      event_vp  - (IN) Port number.
 *      port_vp   - (IN) Port number.
 *      arg1      - (IN) arg1
 *      arg2      - (IN) arg2
 * Returns:
 *      No
 * Notes:
 */
STATIC void
bcmi_esw_portctrl_notify(void *unit_vp, void *event_vp,
                         void *port_vp, void *arg1_vp, void *arg2_vp)
{
    int unit, event, port, arg1, arg2;

    unit = PTR_TO_INT(unit_vp);
    event = PTR_TO_INT(event_vp);
    port = PTR_TO_INT(port_vp);
    arg1 = PTR_TO_INT(arg1_vp);
    arg2 = PTR_TO_INT(arg2_vp);

    (void)soc_event_generate(unit, event, port, arg1, arg2);
}

/*
 * Function:
 *      bcmi_esw_portctrl_update
 * Description:
 *      Get port characteristics from PHY and program MAC to match.
 * Parameters:
 *      unit   - (IN) Device number.
 *      port   - (IN) Port number.
 *      link   - (IN) TRUE - process as link up.
 *                    FALSE - process as link down.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This function cannot hold the PORT_LOCK because it is called
 *      by the linkscan thread.
 */
int
bcmi_esw_portctrl_update(int unit, bcm_gport_t port, int link)
{
#ifdef PORTMOD_SUPPORT
    portctrl_pport_t pport;
    portmod_port_interface_config_t if_config;
    int speed, enable;
    int duplex;
    phymod_autoneg_status_t an_status;
    int rv;
    int is_legacy_phy, skip_spd_sync;

    is_legacy_phy = 0;
    skip_spd_sync = 0;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (!link) {
        /* PHY is down.  Disable the MAC. */
        PORTMOD_IF_ERROR_RETURN 
            ( _bcm_esw_portctrl_enable_set(unit, port, pport,
                                           PORTMOD_PORT_ENABLE_MAC, 0));
        /* PHY link down event */
        rv = (portmod_port_phy_link_down_event(unit, port));
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            LOG_WARN(BSL_LS_BCM_PORT,
                     (BSL_META_U(unit,
                                 "u=%d p=%d portmod_port_phy_link_down_event rv=%d\n"),unit, port, rv));
            return rv;
        }
        return BCM_E_NONE;
    }

    /* PHY link up event */
    rv = (portmod_port_phy_link_up_event(unit, port));
    if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
        LOG_WARN(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "u=%d p=%d portmod_port_phy_link_up_event rv=%d\n"),unit, port, rv));
        return rv;
    }

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_autoneg_status_get(unit, pport, &an_status));

    /*
     * Set MAC speed first, since for GTH ports, this will switch
     * between the 1000Mb/s or 10/100Mb/s MACs.
     */
    if (!IS_HG_PORT(unit, port) || IS_GX_PORT(unit, port)) {
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_interface_config_get(unit, pport, &if_config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
        speed = if_config.speed;

        PORTMOD_IF_ERROR_RETURN
            (portmod_port_duplex_get(unit, pport, &duplex));

        if (IS_HG_PORT(unit, port) && (if_config.speed < 5000)) {
            speed = 0;
        }

#ifdef INCLUDE_FCMAP
        /*
         * In FCMAP mode, the port is operating in FC-Mode. The switch and the
         * system side of the PHY operates in 10G, FullDuplex and
         * Pause enabled mode.
         */
        if (soc_property_port_get(unit, port, spn_FCMAP_ENABLE, 0)) {
            speed = 10000;
            duplex = 1;
        }
#endif
        if (an_status.enabled) {
            /* If AN is enabled, get both "line side speed" and
             * "SERDES speed(multiply number of lanes", then compare them.
             * Otherwise, set LINKUP flag and continue. 
            */
            int line_speed, serdes_speed;

            BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_speed_get(
                                    unit, port, &line_speed));
            PORTMOD_IF_ERROR_RETURN(portmod_port_speed_get(
                                    unit, port, &serdes_speed));
             
            BCM_IF_ERROR_RETURN(
                portmod_port_is_legacy_ext_phy_present(unit, port, &is_legacy_phy));
            if (is_legacy_phy) {
                skip_spd_sync = portmod_port_legacy_is_skip_spd_sync(unit, port);   
            }

            if ((line_speed != serdes_speed) &&  (!skip_spd_sync)) {
                if_config.speed = serdes_speed;
            }
            if_config.flags |= PHYMOD_INTF_F_UPDATE_SPEED_LINKUP;
        }

        /* 1. Assuming Autoneg process is still running.
         * We should not call speed_set function because speed_set involves
         * disable AN, and IP/EP/MMU/TDM reconfiguration.
         * just call _bcm_esw_portctrl_interface_config_set to update interface 
         * 2. For PHY devices that has line and system side operate at different speeds 
         * speed sync between line and system side is not required.
         */   
        if (((if_config.speed != speed) || (((an_status.enabled) && (an_status.locked)))) &&
            (!skip_spd_sync) && link) {
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_enable_get(unit, port, PORTMOD_PORT_ENABLE_MAC,
                                         &enable));
            if (enable) {
                /* Disable MAC */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                                  PORTMOD_PORT_ENABLE_MAC, 0));
            }

            /* reconfigure chip (TDM/IP/EP/MMU) */
            rv = _bcm_esw_portctrl_speed_chip_reconfigure(unit, port, speed);
            if (BCM_FAILURE(rv) && (an_status.enabled)) {
                sal_dpc(bcmi_esw_portctrl_notify,
                        INT_TO_PTR(unit),
                        INT_TO_PTR(SOC_SWITCH_EVENT_AUTONEG_SPEED_ERROR),
                        INT_TO_PTR(port),
                        INT_TO_PTR(speed),
                        INT_TO_PTR(rv));
                return rv;
            }

            if_config.speed = speed;
            PORTMOD_IF_ERROR_RETURN
                (_bcm_esw_portctrl_interface_config_set(unit, port, pport,
                                                        &if_config, 0));

            if (enable) {
                /* Re-enable port */
                BCM_IF_ERROR_RETURN
                    (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                                  PORTMOD_PORT_ENABLE_MAC,
                                                  1));
            }
        }
        
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_duplex_set(unit, pport, duplex));
    } else {
        duplex = 1;
    }

    /*
     * If autonegotiating, check the negotiated PAUSE values, and program
     * MACs accordingly. Link can also be achieved thru parallel detect.
     * In this case, it should be treated as in the forced mode.
     */
    if (an_status.enabled && an_status.locked) {
        bcm_port_ability_t local_advert, remote_advert;
        int tx_pause, rx_pause;
        portmod_pause_control_t pause_control;

        BCM_IF_ERROR_RETURN
            (bcmi_esw_portctrl_ability_advert_get(unit, port,
                                                  &local_advert, NULL));
        BCM_IF_ERROR_RETURN
            (bcmi_esw_portctrl_ability_remote_get(unit, port,
                                                  &remote_advert, NULL));

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

        PORTMOD_IF_ERROR_RETURN
            (portmod_port_pause_control_get(unit, pport, &pause_control));
        pause_control.rx_enable = rx_pause;
        pause_control.tx_enable = tx_pause;
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_pause_control_set(unit, pport, &pause_control));
    }

    /* Enable the MAC */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                      PORTMOD_PORT_ENABLE_MAC, 1));


    /* When a link comes up, hardware will not update the
     * LINK_STATUS register until software has toggled the
     * the LAG_FAILOVER_CONFIG.LINK_STATUS_UP field.
     * When a link goes down, hardware will update the
     * LINK_STATUS register without software intervention.
     */
    /* Toggle link_status_up bit to notify IPIPE on link up */
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_lag_failover_status_toggle(unit, pport));

    return BCM_E_NONE;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_fault_get
 * Description:
 *      Get link fault type.
 * Parameters:
 *      unit   - (IN) Device number.
 *      port   - (IN) Port number.
 *      flags  - (OUT) flags to indicate fault type.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_fault_get(int unit, bcm_gport_t port, uint32 *flags)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int local_fault = 0, remote_fault = 0;
    portmod_local_fault_control_t local_control;
    portmod_remote_fault_control_t remote_control;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);

    /*
     * NOTE:
     * The PortMod functions portmod_port_local/remote_fault_status_get()
     * only return the fault status field values and do not check
     * for the fault disable fields.
     *
     * In order to maintain the same API behavior,
     * we need to call additional PortMod functions to check
     * the fault disable fields.
     */

    rv = portmod_port_local_fault_control_get(unit, pport, &local_control);
    if (PORTMOD_SUCCESS(rv)) {
        if (local_control.enable) {
            rv = portmod_port_local_fault_status_get(unit, pport,
                                                     &local_fault);
        }
    }

    if (PORTMOD_SUCCESS(rv)) {
        rv = portmod_port_remote_fault_control_get(unit, pport,
                                                   &remote_control);
        if (PORTMOD_SUCCESS(rv)) {
            if (remote_control.enable) {
                rv = portmod_port_remote_fault_status_get(unit, pport,
                                                          &remote_fault);
            }
        }
    }

    PORT_UNLOCK(unit);

    *flags = 0;
    if (local_fault) {
        *flags |= BCM_PORT_FAULT_LOCAL;
    }
    if (remote_fault) {
        *flags |= BCM_PORT_FAULT_REMOTE;
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_link_fault_get
 * Description:
 *      Get the fault and remote fault status.
 * Parameters:
 *      unit         - (IN) Device number.
 *      port         - (IN) Port number.
 *      local_fault  - (OUT) Local fault status
 *      remote_fault - (OUT) Remote fault status
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_link_fault_get(int unit, bcm_gport_t port,
                                 int *local_fault, int *remote_fault)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_local_fault_status_get(unit, pport, local_fault);
    if (PORTMOD_SUCCESS(rv)) {
        rv = portmod_port_remote_fault_status_get(unit, pport, remote_fault);
    }
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_loopback_get
 * Purpose:
 *      Recover the current loopback operation for the specified port.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      port     - (IN) Port number.
 *      loopback - (OUT) One of:
 *                       BCM_PORT_LOOPBACK_NONE
 *                       BCM_PORT_LOOPBACK_MAC
 *                       BCM_PORT_LOOPBACK_PHY
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_loopback_get(int unit, bcm_gport_t port, int *loopback)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int i = 0, enable = 0;
    int phy_lane = -1;
    int phyn = 0, sys_side = 0;
    bcm_port_t  local_port = -1;
    uint32 phy_lb = 0;

    portmod_loopback_mode_t portmod_lb_modes[] = {
        portmodLoopbackMacOuter,
        portmodLoopbackPhyGloopPMD,
        portmodLoopbackPhyGloopPCS,
        portmodLoopbackPhyRloopPMD
        /* portmodLoopbackPhyRloopPCS - no support */
    };
    int bcm_lb_modes[] = {
        BCM_PORT_LOOPBACK_MAC,
        BCM_PORT_LOOPBACK_PHY,
        BCM_PORT_LOOPBACK_PHY,
        BCM_PORT_LOOPBACK_PHY_REMOTE,
        BCM_PORT_LOOPBACK_PHY_REMOTE
    };

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_phyn_validate(unit, port,
                                          &local_port, &phyn,
                                          &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (local_port != -1) {
        PORT_LOCK(unit);
        rv = portmod_port_redirect_loopback_get(unit, pport, phyn,
                                                phy_lane, sys_side,
                                                &phy_lb);
        PORT_UNLOCK(unit);
        if (PORTMOD_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_UP(unit, port,
                            "Redirect loopback get failed: p=%d, rv=%d"),
                        port, rv));
            return rv;
        }
    }

    *loopback = BCM_PORT_LOOPBACK_NONE;

    for (i = 0; i < COUNTOF(portmod_lb_modes); i++) {
        PORT_LOCK(unit);
        rv = portmod_port_loopback_get(unit, pport,
                                       portmod_lb_modes[i], &enable);
        PORT_UNLOCK(unit);

        if (PORTMOD_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_PORT,
                        (BSL_META_UP(unit, port,
                                     "Loopback get failed: p=%d, rv=%d"),
                        port, rv));
            return rv;
        }

        if (enable) {
            *loopback = bcm_lb_modes[i];
            break;
        }
    }

    if (*loopback != BCM_PORT_LOOPBACK_MAC && phy_lb) {
        *loopback = BCM_PORT_LOOPBACK_PHY;
    }

    return BCM_E_NONE;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_loopback_set
 * Purpose:
 *      Set loopback mode for the specified port.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      loopback - one of:
 *              BCM_PORT_LOOPBACK_NONE
 *              BCM_PORT_LOOPBACK_MAC
 *              BCM_PORT_LOOPBACK_PHY
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_loopback_set(int unit, bcm_gport_t port, int loopback)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE, link = TRUE;
    portctrl_pport_t pport;
    int phy_lane = -1;
    int phyn = 0, sys_side = 0;
    int flags = 0, is_loopback_set = 0;
    int mac_lpbk = 0, phy_lpbk = 0, rmt_lpbk = 0;
    bcm_port_t local_port = -1;
    int tmp_flags=0;
    soc_persist_t *sop = SOC_PERSIST(unit);

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_phyn_validate(unit, port,
                                          &local_port, &phyn,
                                          &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

#ifdef BCM_RCPU_SUPPORT
    if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
#endif /* BCM_RCPU_SUPPORT */

    /* get current loopback state.  Do nothing if the current state matches requested state */
    if (local_port == -1) {
        rv = portmod_port_loopback_get(unit, pport, portmodLoopbackMacOuter,    &mac_lpbk); 
        if (PORTMOD_FAILURE(rv)) return(rv);
 
        rv = portmod_port_loopback_get(unit, pport, portmodLoopbackPhyGloopPCS, &phy_lpbk);
        if (PORTMOD_FAILURE(rv)) return(rv);

        rv = portmod_port_loopback_get(unit, pport, portmodLoopbackPhyRloopPMD, &rmt_lpbk);
        if (PORTMOD_FAILURE(rv)) return(rv);

        if (((loopback == BCM_PORT_LOOPBACK_MAC) && mac_lpbk) ||
            ((loopback == BCM_PORT_LOOPBACK_PHY) && phy_lpbk) ||
            ((loopback == BCM_PORT_LOOPBACK_PHY_REMOTE) && rmt_lpbk) ||
            ((loopback == BCM_PORT_LOOPBACK_NONE) && !mac_lpbk && !phy_lpbk && !rmt_lpbk)) {

            /* current state matches with requested state - dont need to do anything */
            LOG_VERBOSE(BSL_LS_BCM_PORT, (BSL_META_UP(unit, port, "Port%d : Loopback Command Skip (State Match)\n"), port));
            is_loopback_set = 1;
        }
    }

    PORTMOD_PORT_ENABLE_PHY_SET(flags);
    PORT_LOCK(unit);
    rv = portmod_port_enable_get(unit, pport, flags, &link);
    PORT_UNLOCK(unit);

    if (PORTMOD_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                 "Get port enable failed: p=%d, rv=%d"),
                    port, rv));
    }

    if (TRUE == link) {
        PORT_LOCK(unit);

        if (SOC_PBMP_MEMBER(sop->lc_pbm_override_ports, port)) {
            if(!SOC_PBMP_MEMBER(sop->lc_pbm_override_link, port)) {
                link = FALSE;
            }
        }

        PORT_UNLOCK(unit);
    }

    /*
     * Always force link before changing hardware to avoid
     * race with the linkscan thread.
     */
    if (!(loopback == BCM_PORT_LOOPBACK_NONE)) {
        rv = _bcm_esw_link_force(unit, 0 /*flags*/, port, TRUE, FALSE);

        if(rv == BCM_E_NONE) {
            /* Force the update function to be called so the state is
               updated */
            if (_soc_linkscan_phy_flags_test(unit, port, PHY_FLAGS_EXTERNAL_PHY)) {
                rv = bcm_esw_port_update(unit, port, TRUE);
            }
        }
    }

    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    if (!is_loopback_set) {
        PORT_LOCK(unit);

        if (local_port == -1) {
            int lpbk_type = portmodLoopbackCount;

            /* clear if any loopback was set prior */
            if (BCM_SUCCESS(rv) && mac_lpbk) {
                rv = portmod_port_loopback_set(unit, pport,
                                               portmodLoopbackMacOuter, 0);
            }
            if (BCM_SUCCESS(rv) && phy_lpbk) {
                rv = portmod_port_loopback_set(unit, pport,
                                               portmodLoopbackPhyGloopPCS, 0);
            }
            if (BCM_SUCCESS(rv) && rmt_lpbk) {
                rv = portmod_port_loopback_set(unit, pport,
                                               portmodLoopbackPhyRloopPMD, 0);
            }

            if (BCM_FAILURE(rv)) {
                PORT_UNLOCK(unit);
                return (rv);
            }

            if (loopback == BCM_PORT_LOOPBACK_MAC) {
                lpbk_type =  portmodLoopbackMacOuter;
            } else if (loopback == BCM_PORT_LOOPBACK_PHY){
                lpbk_type =  portmodLoopbackPhyGloopPCS;
            } else if (loopback == BCM_PORT_LOOPBACK_PHY_REMOTE) {
                lpbk_type = portmodLoopbackPhyRloopPMD;
            }

            if (lpbk_type != portmodLoopbackCount) {
                rv = portmod_port_loopback_set(unit, pport, lpbk_type, 1);
            }
        } else {
            rv = portmod_port_redirect_loopback_set(unit, pport,
                            phyn, phy_lane, sys_side,
                            (loopback == BCM_PORT_LOOPBACK_PHY)? 1 : 0);
        }
        PORT_UNLOCK(unit);  /* unlock before link call */
    }

    flags = 0;
    PORTMOD_PORT_ENABLE_PHY_SET(flags);
    PORTMOD_PORT_ENABLE_MAC_SET(flags);

    if ((loopback == BCM_PORT_LOOPBACK_NONE) || PORTMOD_FAILURE(rv)) {
        _bcm_esw_link_force(unit, 0 /*flags*/, port, FALSE, DONT_CARE);

        if ((FALSE == link) && (loopback == BCM_PORT_LOOPBACK_NONE)) {
            PORT_LOCK(unit);
            rv = _bcm_esw_portctrl_enable_set(unit, port, pport, flags, 0);
            PORT_UNLOCK(unit);
        } else {

            /* phy tx need to be re-enable when the port is enabled and
               clearing the loopback. */
            tmp_flags = 0;
            PORTMOD_PORT_ENABLE_PHY_SET(tmp_flags);
            PORTMOD_PORT_ENABLE_TX_SET(tmp_flags);
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_enable_set(unit, port, tmp_flags, 1));
        }
    } else {
        PORT_LOCK(unit);
        rv = _bcm_esw_portctrl_enable_set(unit, port, pport, flags, 1);

        /* if the port is disabled and apply loopback, phy tx need to
           be disabled. */
        if(FALSE == link){
            tmp_flags = 0;
            PORTMOD_PORT_ENABLE_PHY_SET(tmp_flags);
            PORTMOD_PORT_ENABLE_TX_SET(tmp_flags);
            PORTMOD_IF_ERROR_RETURN
                (portmod_port_enable_set(unit, port, tmp_flags, 0));
        }

        PORT_UNLOCK(unit);

        if (PORTMOD_SUCCESS(rv)) {
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
            if (loopback == BCM_PORT_LOOPBACK_MAC) {
                rv = _bcm_esw_link_force(unit, 0 /*flags*/, port, TRUE, TRUE);
            } else {
                rv = _bcm_esw_link_force(unit, 0 /*flags*/, port, TRUE, link);
            }
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "bcm_port_loopback_set: p=%d lb=%d rv=%d\n"),
              port, loopback, rv));

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_speed_get
 * Purpose:
 *      Getting the speed of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      If port is in MAC loopback, the speed of the loopback is returned.
 */
int
bcmi_esw_portctrl_speed_get(int unit, bcm_gport_t port, int *speed)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE, valid = 0;
    portctrl_pport_t  pport;
    portmod_port_interface_config_t portmod_if_config;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN(portmod_port_is_valid(unit, pport, &valid));
    if (!valid) {
        return BCM_E_PORT;
    }

    PORT_LOCK(unit);
    rv = portmod_port_interface_config_get(unit, pport, &portmod_if_config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        *speed = portmod_if_config.speed;

        if (IS_HG_PORT(unit, port) && *speed < 5000) {
            *speed = 0;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Get port speed: p=%d speed=%d rv=%d\n"),
              port, BCM_SUCCESS(rv) ? *speed : 0, rv));

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_speed_max
 * Purpose:
 *      Getting the maximum speed of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_speed_max(int unit, bcm_gport_t port, int *speed)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    bcm_port_ability_t ability;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    sal_memset(&ability, 0, sizeof(bcm_port_ability_t));

    if (speed == NULL) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    rv = bcmi_esw_portctrl_ability_get(unit, port, &ability, NULL);

    if (BCM_SUCCESS(rv)) {
        *speed = BCM_PORT_ABILITY_SPEED_MAX(ability.speed_full_duplex |
                                            ability.speed_half_duplex);

        if (IS_HG_PORT(unit, port)) {
            if (SOC_INFO(unit).port_speed_max[port]) {
                switch (*speed) {
                case 10000:
                case 20000:
                case 40000:
                case 100000:
                case 120000:
                    *speed = SOC_INFO(unit).port_speed_max[port];
                    break;
                default:
                    break;
                }
            }
        } else {
            switch (*speed) {
            case 11000:
            case 21000:
            case 42000:
            case 106000:
            case 127000:
                *speed = SOC_INFO(unit).port_speed_max[port];
                break;
            case 13000:
            case 16000:
                if(SOC_IS_TITAN2PLUS(unit)){
                    *speed = SOC_INFO(unit).port_speed_max[port];
                }
                break;
            default:
                break;
            }
        }

    } else {
        *speed = 0;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Max port speed: p=%d speed=%d rv=%d\n"),
              port, *speed, rv));

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

#ifdef PORTMOD_SUPPORT

/*
 * Function:
 *      _bcm_esw_portctrl_speed_validate
 * Purpose:
 *      Validate the speed change can be supported
 * Parameters:
 *      unit     - SOC unit number
 *      port     - bcm_gport_t port number
 *      speed    - port speed
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_portctrl_speed_validate(int unit, bcm_gport_t port, int speed)
{
    bcm_port_ability_t port_ability, requested_ability;

    sal_memset(&port_ability, 0, sizeof(bcm_port_ability_t));
    sal_memset(&requested_ability, 0, sizeof(bcm_port_ability_t));

    BCM_IF_ERROR_RETURN
        (bcmi_esw_portctrl_ability_get(unit, port, &port_ability, NULL));

    requested_ability.speed_full_duplex = SOC_PA_SPEED(speed);
    requested_ability.speed_half_duplex = SOC_PA_SPEED(speed);

    if (((port_ability.speed_full_duplex &
          requested_ability.speed_full_duplex) == 0) &&
        ((port_ability.speed_half_duplex &
          requested_ability.speed_half_duplex) == 0)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                 "Port %d doesn't support %d Mbps speed.\n"),
                    port, speed));
        return BCM_E_CONFIG;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_portctrl_disable_autoneg
 * Purpose:
 *      Disable Autoneg
 * Parameters:
 *      unit     - SOC unit number
 *      pport    - port type for portmod function
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_portctrl_disable_autoneg(int unit, portctrl_pport_t  pport)
{
    int rv;
    phymod_autoneg_control_t an;

    sal_memset(&an, 0, sizeof(phymod_autoneg_control_t));

    PORT_LOCK(unit);
    rv = portmod_port_autoneg_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an);
    if (PORTMOD_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }

    an.enable = FALSE;

    rv = portmod_port_autoneg_set(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an);
    if (PORTMOD_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    PORT_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_portctrl_speed_chip_reconfig
 * Purpose:
 *      Reconfigure chip(PGW/TDM/IP/EP/MMU) for speed set change
 * Parameters:
 *      unit     - SOC unit number
 *      port     - bcm_port_t port number
 *      speed    - port speed
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_portctrl_speed_chip_reconfigure(int unit, bcm_port_t port, int speed)
{
    int rv = BCM_E_NONE;

    PORT_LOCK(unit);
    if (BCM_ESW_PORT_DRV(unit)!= NULL) {
        if (BCM_ESW_PORT_DRV(unit)->resource_speed_set != NULL) {
            rv = BCM_ESW_PORT_DRV(unit)->resource_speed_set(unit, port,
                                                            speed);
        }
    }
    PORT_UNLOCK(unit);

    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                 "Set speed for chip resource failed\n")));
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_portctrl_speed_interface_config_set
 * Purpose:
 *      Change interface for speed.
 * Parameters:
 *      unit     - SOC unit number
 *      port     - bcm_port_t port number
 *      pport    - port number for portmod function
 *      speed    - port speed
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_portctrl_speed_interface_config_set(int unit, bcm_gport_t port,
                                             portctrl_pport_t pport, int speed)
{
    int rv;
    portmod_port_interface_config_t portmod_if_config;
    soc_port_if_t sys_interface;
    soc_port_if_t default_interface;
    bcm_gport_t   gport;

    PORT_LOCK(unit);

    rv = portmod_port_interface_config_get(unit, pport, &portmod_if_config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);
    if (PORTMOD_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }

    /* When changing the speed, the existing interface type of the
     * port may not be valid anymore. Need to retrieve the default
     * interface type based on the new speed and update 
     * portmod_if_config.
     */
    if (portmod_if_config.speed != speed) {
        portmod_if_config.speed = speed;
        default_interface = SOC_PORT_IF_NULL;
        SOC_IF_ERROR_RETURN(
                portmod_port_default_interface_get
                    (unit, port, &portmod_if_config, &default_interface));
        if (default_interface != SOC_PORT_IF_NULL) {
            portmod_if_config.interface = default_interface;
        } else {
            SOC_IF_ERROR_RETURN(
                    portmod_common_default_interface_get(&portmod_if_config));
        }
    }


    if (PORTMOD_SUCCESS(rv)) {
        rv = _bcm_esw_portctrl_interface_config_set(unit, port, pport,
                                                    &portmod_if_config, 1);
    }

    sys_interface = soc_property_port_get(unit, port,
                                spn_PHY_SYS_INTERFACE, SOC_PORT_IF_COUNT);

    if (sys_interface != SOC_PORT_IF_COUNT) {
        BCM_PHY_GPORT_PHYN_SYS_SIDE_PORT_SET(gport, 0, port);
        bcmi_esw_portctrl_phy_control_set(unit, gport, SOC_PHY_CONTROL_INTERFACE,
                                            sys_interface);
    }

    PORT_UNLOCK(unit);

    if (PORTMOD_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                "Set port speed failed: p=%d speed=%d rv=%d\n"),
                    port, speed, rv));
    }

    return rv;
}
#endif

/*
 * Function:
 *      bcmi_esw_portctrl_speed_set
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_speed_set(int unit, bcm_gport_t port, int speed)
{
#ifdef PORTMOD_SUPPORT
    portctrl_pport_t  pport;
    int cur_speed;
    int loopback;
    int enable;
    int rv = SOC_E_NONE;

    /* If current speed is different from target speed,
       need to call full speed_set sequence which includes IP/EP/MMU/TDM reprogramming
       If current speed is same as target speed, just
       need to disable Autoneg and call interface_config_set to apply interface change */
#define IS_FULL_SPEED_SET_REQUIRED(cur_speed, new_speed) \
        ((cur_speed) != (new_speed))

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    /* check inactive port */
    if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), port)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                 "Port %d is inactive port\n"), port));
        return BCM_E_NONE;
    }

    /* if speed is 0, set the port speed to max */
    if (speed == 0) {
        BCM_IF_ERROR_RETURN
            (bcmi_esw_portctrl_speed_max(unit, port, &speed));
    }

    /* get current speed */
    BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_speed_get(unit, port, &cur_speed));


    if (IS_FULL_SPEED_SET_REQUIRED(cur_speed, speed)) {
        /* validation : speed change is supported */
        BCM_IF_ERROR_RETURN(_bcm_esw_portctrl_speed_validate(unit, port, speed));

        /* clear loopback */
        BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_loopback_get(unit,
                                            port, &loopback));
        if (loopback != BCM_PORT_LOOPBACK_NONE) {
            BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_loopback_set(unit,
                                            port,
                                            BCM_PORT_LOOPBACK_NONE));
        }
        BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_enable_get(unit, port, &enable));
        if (enable == TRUE) {
            /* disable MAC and PHY */
            BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_enable_set(unit, port, FALSE));
        }

        /* disable AN */
        BCM_IF_ERROR_RETURN(_bcm_esw_portctrl_disable_autoneg(unit, pport));

        /* reconfigure chip (PGW/TDM/IP/EP/MMU) */
        rv = _bcm_esw_portctrl_speed_chip_reconfigure(unit, port, speed);
        if (BCM_SUCCESS(rv)) {
            /* reconfigure MAC & PHY */
            BCM_IF_ERROR_RETURN(_bcm_esw_portctrl_speed_interface_config_set(
                                        unit, port, pport, speed));
        }
        if (enable == TRUE) {
            /* Restore port's enable state based on what was read prior to
               setting speed */
            BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_enable_set(unit, port, TRUE));
        }
        /* restore loopback */
        if (loopback != BCM_PORT_LOOPBACK_NONE) {
            BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_loopback_set(unit,
                                            port,
                                            loopback));
        }
    } else {
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_enable_get(unit, port, PORTMOD_PORT_ENABLE_MAC, &enable));

        /* No full sequence is not required. If speed is same,
           just disable autoneg and apply interface_config_set */
        if (enable == TRUE) {
            /* Disable port */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                              PORTMOD_PORT_ENABLE_MAC, 0));
        }

        /* Autoneg should be disabled after disabling the MAC */
        BCM_IF_ERROR_RETURN(_bcm_esw_portctrl_disable_autoneg(unit, pport));

        /* reconfigure MAC & PHY */
        BCM_IF_ERROR_RETURN(_bcm_esw_portctrl_speed_interface_config_set(
                                    unit, port, pport, speed));
        if (enable == TRUE) {
            /* Restore port's enable state. */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_portctrl_enable_set(unit, port, pport,
                                              PORTMOD_PORT_ENABLE_MAC, 1));
        }
    }
    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_stk_my_modid_set
 * Purpose:
 *      Set the module-id in the Port block
 * Parameters:
 *      unit  - SOC unit#
 *      port - port #
 *      my_modid - the value to set
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_stk_my_modid_set(int unit, bcm_gport_t port, int my_modid)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (IS_HG_PORT(unit, port)) {
        PORT_LOCK(unit);
        rv = portmod_port_modid_set(unit, pport, my_modid);
        PORT_UNLOCK(unit);
    }

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_pause_get
 * Purpose:
 *      Get the TX and RX settings for pause frames.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      port     - (IN) Port number.
 *      pause_tx - (OUT) Boolean value.
 *      pause_rx - (OUT) Boolean value.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_pause_get(int unit, bcm_gport_t port,
                            int *pause_tx, int *pause_rx)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    portmod_pause_control_t pause_control;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_pause_control_get(unit, pport, &pause_control);
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        *pause_tx = pause_control.tx_enable;
        *pause_rx = pause_control.rx_enable;
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_pause_set
 * Purpose:
 *      Enable or disable transmission of pause frames and
 *      honoring received pause frames on a port.
 * Parameters:
 *      unit     - (IN) Unit number.
 *      port     - (IN) Port number.
 *      pause_tx - (IN) 0 to disable transmission of pauseframes, 1 to enable.
 *      pause_rx - (IN) 0 to ignore received pause frames, 1 to honor
 *                      received pause frames.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_pause_set(int unit, bcm_gport_t port,
                            int pause_tx, int pause_rx)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    portmod_pause_control_t pause_control;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);

    rv = portmod_port_pause_control_get(unit, pport, &pause_control);
    if (PORTMOD_SUCCESS(rv)) {
        pause_control.rx_enable = pause_rx;
        pause_control.tx_enable = pause_tx;
        rv = portmod_port_pause_control_set(unit, pport, &pause_control);
    }

    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_pause_addr_get
 * Purpose:
 *      Get the source address for transmitted PAUSE frames.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      mac    - (OUT) MAC address sent with pause frames.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_pause_addr_get(int unit, bcm_gport_t port, bcm_mac_t mac)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    /* sal_mac_addr_t and bcm_mac_t typedefs definitions are the same */
    PORT_LOCK(unit);
    rv = portmod_port_tx_mac_sa_get(unit, pport, mac);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_pause_addr_set
 * Purpose:
 *      Set the source address for transmitted PAUSE frames.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      mac    - (IN) Station MAC address used for pause frames.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_pause_addr_set(int unit, bcm_gport_t port, bcm_mac_t mac)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    /* sal_mac_addr_t and bcm_mac_t typedefs definitions are the same */
    PORT_LOCK(unit);
    rv = portmod_port_tx_mac_sa_set(unit, pport, mac);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_eee_enable_get
 * Purpose:
 *      Retrieve eee enable field.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Port number.
 *      enable       - (OUT) Returns eee enable field.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_eee_enable_get(int unit, bcm_gport_t port, int *enable)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;
    portmod_eee_t eee;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_eee_get(unit, pport, &eee);
    *enable = eee.enable;
    PORT_UNLOCK(unit);

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_eee_enable_set
 * Purpose:
 *      Set the eee enable field if different from what is to be set.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Port number.
 *      enable       - (IN) eee enable field.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_eee_enable_set(int unit, bcm_gport_t port, int enable)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;
    portmod_eee_t eee;

    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_eee_get(unit, pport, &eee);
    if (PORTMOD_SUCCESS(rv)) {
        if (eee.enable != enable) {
            eee.enable = enable;
            rv = portmod_port_eee_set(unit, pport, &eee);
        }
    }
    PORT_UNLOCK(unit);

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_master_get
 * Purpose:
 *      Getting the master status of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ms - (OUT) BCM_PORT_MS_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */

int
bcmi_esw_portctrl_master_get(int unit, bcm_gport_t port, int *ms)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    portmod_dispatch_type_t pm_type;
    int pm_type_supported = 0;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    rv = portmod_port_pm_type_get(unit, port, &port, &pm_type);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /*
     * portmod_port_master_get() fetches the master_mode regsiter value ONLY WHEN
     * there is a legacy ext PHY attaching to a port. Currently there is no
     * master_mode_get API implemented for PHYMOD-based int/ext PHY. When a portmod
     * port is NOT attaching to an ext PHY, it simply updates the master mode as
     * slave since the internal PHY only operates in the slave mode. When the portmod
     * port is attaching to a PHYMOD-based ext PHY, the master mode will be updated as
     * BCM_PORT_MS_NONE to reflect the unknown status.
     */

#ifdef PORTMOD_PM4X10_SUPPORT
    if (pm_type == portmodDispatchTypePm4x10
#ifdef PORTMOD_PM4X10TD_SUPPORT 
        || pm_type == portmodDispatchTypePm4x10td
#endif /* PORTMOD_PM4X10TD_SUPPORT */
       ) {
        rv = portmod_port_master_get(unit, port, ms);
        pm_type_supported = 1;
    }
#endif /* PORTMOD_PM4X10_SUPPORT */

#ifdef PORTMOD_PM4X25_SUPPORT
    if (pm_type == portmodDispatchTypePm4x25
#ifdef PORTMOD_PM4X25TD_SUPPORT
        || pm_type == portmodDispatchTypePm4x25td
#endif /* PORTMOD_PM4X25TD_SUPPORT */
       ) {
        rv = portmod_port_master_get(unit, port, ms);
        pm_type_supported = 1;
    }
#endif /* PORTMOD_PM4X25_SUPPORT */

    if (!pm_type_supported) {
        *ms = BCM_PORT_MS_NONE;
    }

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_medium_config_set
 * Description:
 *      Set the medium config of a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 which is currently selected
 *      config   - Medium config
 * Return Value:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_medium_config_set(int unit, bcm_gport_t port, bcm_port_medium_t medium,
                                    bcm_phy_config_t *config)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_medium_config_set(unit, pport, medium, config);
    PORT_UNLOCK(unit);
    
    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_medium_config_get
 * Description:
 *      Get the medium config of a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 which is currently selected
 *      config   - Medium config
 * Return Value:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_medium_config_get(int unit, bcm_gport_t port, bcm_port_medium_t medium,
                                    bcm_phy_config_t *config)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int ext_phy_present;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    /* PORTMOD currently only supports medium config get for ports with legacy ext PHY */
    SOC_IF_ERROR_RETURN(portmod_port_is_legacy_ext_phy_present(unit, pport, &ext_phy_present));
    if (!ext_phy_present) { 
        return BCM_E_UNAVAIL; 
    }

    PORT_LOCK(unit);
    rv = portmod_port_medium_config_get(unit, pport, medium, config);
    PORT_UNLOCK(unit);
    
    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_port_medium_get
 * Description:
 *      Get the current medium used by a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 which is currently selected
 * Return Value:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_medium_get(int unit, bcm_gport_t port,
                             bcm_port_medium_t *medium)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portmod_port_diag_info_t info;
    portctrl_pport_t pport;
    int ext_phy_present;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    memset(&info, 0, sizeof(info)); /* Keep Coverity happy */
    portmod_port_diag_info_t_init(unit, &info);

    PORT_LOCK(unit);

    rv = portmod_port_diag_info_get(unit, pport, &info);

    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        *medium = info.medium;
    }

    /* dig further if there is an ext PHY present and medium type is not yet found */
    rv = portmod_port_is_legacy_ext_phy_present(unit, pport, &ext_phy_present);
    if (ext_phy_present && *medium == SOC_PORT_MEDIUM_NONE){
        PORT_LOCK(unit);
        portmod_port_medium_get(unit, pport, medium);
        PORT_UNLOCK(unit);
    }

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_master_set
 * Purpose:
 *      Setting the master status for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ms - BCM_PORT_MS_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Ignored if not supported on port.
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */
int
bcmi_esw_portctrl_master_set(int unit, bcm_gport_t port, int ms)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    pbmp_t pbm;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

#ifdef BCM_RCPU_SUPPORT
    if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
#endif /* BCM_RCPU_SUPPORT */


    SOC_PBMP_CLEAR(pbm);
    SOC_PBMP_PORT_ADD(pbm, port);
    (void)bcm_esw_link_change(unit, pbm);

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */

}


/*
 * Function:
 *      bcmi_portctrl_interface_get
 * Purpose:
 *      Getting the interface type of a port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      intf - (OUT) BCM_PORT_IF_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */

int
bcmi_esw_portctrl_interface_get(int unit, bcm_gport_t port, bcm_port_if_t *intf)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portmod_port_interface_config_t port_if_cfg;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (!SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port)) {
        return BCM_E_PORT;
    }

    portmod_port_interface_config_t_init(unit, &port_if_cfg);

    PORT_LOCK(unit);
    rv = portmod_port_interface_config_get(unit, pport, &port_if_cfg, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        *intf = port_if_cfg.interface;
    }

    return rv;
#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */

}

/*
 * Function:
 *      bcmi_portctrl_interface_set
 * Purpose:
 *      Setting the interface type for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      if - BCM_PORT_IF_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */
int
bcmi_esw_portctrl_interface_set(int unit, bcm_gport_t port, bcm_port_if_t intf)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    char *str = "";
    pbmp_t pbm;
    portmod_port_interface_config_t port_if_cfg;
    portctrl_pport_t pport;
    _bcm_port_info_t *port_info = NULL;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (!SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_RCPU_SUPPORT
    if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
#endif /* BCM_RCPU_SUPPORT */

    portmod_port_interface_config_t_init(unit, &port_if_cfg);

    PORT_LOCK(unit);
    rv = portmod_port_interface_config_get(unit, pport, &port_if_cfg, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);

    if (PORTMOD_SUCCESS(rv)) {
        
        /* Set interface type */
        port_if_cfg.interface = intf;
        port_if_cfg.flags     = PHYMOD_INTF_F_INTF_PARAM_SET_ONLY ;

        rv = _bcm_esw_portctrl_interface_config_set(unit, port, pport,
                                                    &port_if_cfg, 0);
        PORT_UNLOCK(unit);

        /* Unlock before link call */
        if (PORTMOD_SUCCESS(rv)) {
            SOC_PBMP_CLEAR(pbm);
            SOC_PBMP_PORT_ADD(pbm, port);
            (void)bcm_esw_link_change(unit, pbm);
            if (soc_property_get(unit, spn_SAME_SPEED_INTF_DO_NOT_OVERWRITE,
                                 (SAL_BOOT_BCMSIM || SAL_BOOT_XGSSIM))) {
                _bcm_port_info_access(unit, port, &port_info);
                port_info->intf = intf;
            }
        } else {
            str = "set";
        }
    } else {
        PORT_UNLOCK(unit);
        str = "get";
    }

    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_UP(unit, port,
                    "Interface_%s failed:%s\n"), str, bcm_errmsg(rv)));
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_mdix_get
 * Description:
 *      Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - (Out) One of:
 *              BCM_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              BCM_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              BCM_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              BCM_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      BCM_E_UNAVAIL - feature unsupported by hardware
 *      BCM_E_XXX - other error
 */
int
bcmi_esw_portctrl_mdix_get(int unit, bcm_gport_t port, bcm_port_mdix_t *mode)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int ext_phy_present = 0;
    char *str = "";

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_is_legacy_ext_phy_present(unit, pport, &ext_phy_present);

    if (PORTMOD_SUCCESS(rv)) {
        if (ext_phy_present) {
            rv = portmod_port_ext_phy_mdix_get(unit, pport, mode);

            str = "portmod_port_ext_phy_mdix_get";
        } else {
            /* MDIX is supported for external phys only, so we return a fixed
             * value for internal phys
             */
            *mode = SOC_PORT_MDIX_NORMAL;
        }
    } else {
        str = "portmod_port_is_legacy_ext_phy_present";
    }

    PORT_UNLOCK(unit);
    if (PORTMOD_FAILURE(rv)) {

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port, "MDIX get: %s failed:\n"
                    "port=%d, pport=%d, rv=%d\n"), str, port, pport, rv));
    }

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_mdix_set
 * Description:
 *      Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - One of:
 *              BCM_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              BCM_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              BCM_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      BCM_E_UNAVAIL - feature unsupported by hardware
 *      BCM_E_XXX - other error
 */
int
bcmi_esw_portctrl_mdix_set(int unit, bcm_gport_t port, bcm_port_mdix_t mode)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int ext_phy_present = 0;
    char *str = "";

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_is_legacy_ext_phy_present(unit, pport, &ext_phy_present);

    if (PORTMOD_SUCCESS(rv)) {
        if (ext_phy_present) {
            rv = portmod_port_ext_phy_mdix_set(unit, pport, mode);

            str = "portmod_port_ext_phy_mdix_set failed:";
        } else {
            /* The behavior of this function should match legacy mdix API's,
             * so if mdix mode is other than normal, we return error
             */
            if (mode != SOC_PORT_MDIX_NORMAL) {
                rv = BCM_E_UNAVAIL;
                str = "mode is not 'normal' ";
            }
        }

        /* For internal phys, we do not program h/w; we return success */
    } else {
        str = "portmod_port_is_legacy_ext_phy_present failed:";
    }

    PORT_UNLOCK(unit);
    if (PORTMOD_FAILURE(rv)) {

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port, "MDIX set: %s\nport=%d,"
                    " pport=%d, rv=%d\n"), str, port, pport, rv));
    }

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_mdix_status_get
 * Description:
 *      Get the current MDIX status on a port/PHY
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 *      status  - (OUT) One of:
 *              BCM_PORT_MDIX_STATUS_NORMAL
 *                      Straight connection
 *              BCM_PORT_MDIX_STATUS_XOVER
 *                      Crossover has been performed
 * Return Value:
 *      BCM_E_UNAVAIL - feature unsupported by hardware
 *      BCM_E_XXX - other error
 */
int
bcmi_esw_portctrl_mdix_status_get(int unit, bcm_gport_t port,
                                  bcm_port_mdix_status_t *status)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int ext_phy_present = 0;
    char *str = "";

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_is_legacy_ext_phy_present(unit, pport, &ext_phy_present);

    if (PORTMOD_SUCCESS(rv)) {

        /* MDIX is supported for external phys only */
        if (ext_phy_present) {
            rv = portmod_port_ext_phy_mdix_status_get(unit, pport, status);

            str = "portmod_port_ext_phy_mdix_status_get";
        } else {
            *status = SOC_PORT_MDIX_STATUS_NORMAL;
        }
    } else {
        str = "portmod_port_is_legacy_ext_phy_present";
    }

    PORT_UNLOCK(unit);
    if (PORTMOD_FAILURE(rv)) {

        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port, "MDIX status get: %s failed:\n"
                    "port=%d, pport=%d, rv=%d\n"), str, port, pport, rv));
    }
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_phy_get
 * Description:
 *      General PHY register read
 * Parameters:
 *      unit - Device number
 *      port - Port number or PHY MDIO address (refer BCM_PORT_PHY_NOMAP)
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - (OUT) Data that was read
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_get(int unit, bcm_gport_t port, uint32 flags,
                          uint32 phy_reg_addr, uint32 *phy_data)
{
#ifdef PORTMOD_SUPPORT
    uint16  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint32 reg_flag;
    int    rv;
    portctrl_pport_t pport=0;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));
        if(!(SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port))) {
            return BCM_E_PORT;
        }
    }

    if (flags & (BCM_PORT_PHY_I2C_DATA8 | BCM_PORT_PHY_I2C_DATA16 | BCM_PORT_PHY_PVT_DATA)) {
        PORT_LOCK(unit);
        rv = portmod_port_phy_reg_read(unit, pport, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
        return rv;
    }

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & BCM_ESW_PORTCTRL_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }

        phy_reg_addr &= ~BCM_ESW_PORTCTRL_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = portmod_port_phy_reg_read(unit, pport, 0, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = (uint16) port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            PORT_LOCK(unit);
            phy_id = portmod_port_to_phyaddr_int(unit, pport);
            PORT_UNLOCK(unit);
        } else {
            PORT_LOCK(unit);
            phy_id = portmod_port_to_phyaddr(unit, pport);
            PORT_UNLOCK(unit);
        }

        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad,
                                  phy_reg, &phy_rd_data);

        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
        }
        PORT_UNLOCK(unit);

        if (BCM_SUCCESS(rv)) {
           *phy_data = phy_rd_data;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "portctrl_phy_get: u=%d p=%d flags=0x%08x "
                         "phy_reg=0x%08x, phy_data=0x%08x, rv=%d\n"),
                         unit, port, flags, phy_reg_addr, *phy_data, rv));

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_phy_multi_get
 * Description:
 *      General PHY register read
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Phy's read-specific flags
 *      dev_addr - Device address on the PHY's bus (ex. I2C addr)
 *      offset - Offset within device
 *      max_size - Requested data size
 *      data - Data buffer
 *      actual_size - Received data size
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_multi_get(int unit, bcm_gport_t port, uint32 flags,
                                uint32 dev_addr, uint32 offset, int max_size,
                                uint8 *data, int *actual_size)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;
    portmod_multi_get_t phy_reg_read;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    /* Initialize and fill the structure */
    portmod_multi_get_t_init(unit, &phy_reg_read);

    phy_reg_read.flags = flags;
    phy_reg_read.dev_addr = dev_addr;
    phy_reg_read.offset = offset;
    phy_reg_read.max_size = max_size;
    phy_reg_read.data = data;
    phy_reg_read.actual_size = (uint32 *)actual_size;

    PORT_LOCK(unit);
    rv = portmod_port_multi_get(unit, pport, &phy_reg_read);
    PORT_UNLOCK(unit);

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_phy_modify
 * Description:
 *      General PHY register modify
 * Parameters:
 *      unit - Device number
 *      port - Port number  or PHY MDIO address (refer BCM_PORT_PHY_NOMAP)
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_reg_addr - PHY internal register address
 *      phy_data - Data to write
 *      phy_mask - Bits to modify using phy_data
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_modify(int unit, bcm_gport_t port, uint32 flags,
                             uint32 phy_reg_addr, uint32 phy_data,
                             uint32 phy_mask)
{
#ifdef PORTMOD_SUPPORT
    uint16 phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;
    portctrl_pport_t pport=0;


    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));
        if(!(SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port))) {
            return BCM_E_PORT;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "portctrl_phy_modify: u=%d p=%d flags=0x%08x "
                         "phy_reg=0x%08x phy_data=0x%08x phy_mask=0x%08x\n"),
                         unit, port, flags, phy_reg_addr, phy_data, phy_mask));

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & BCM_ESW_PORTCTRL_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }

        phy_reg_addr &= ~BCM_ESW_PORTCTRL_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = portmod_port_phy_reg_write(unit, pport, 0, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = (uint16) port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            PORT_LOCK(unit);
            phy_id = portmod_port_to_phyaddr_int(unit, pport);
            PORT_UNLOCK(unit);
        } else {
            PORT_LOCK(unit);
            phy_id = portmod_port_to_phyaddr(unit, pport);
            PORT_UNLOCK(unit);
        }

        phy_wr_data = (uint16) (phy_data & phy_mask & 0xffff);
        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad,
                                  phy_reg, &phy_rd_data);
            phy_wr_data |= (phy_rd_data & ~phy_mask);
            rv = soc_miimc45_write(unit, phy_id, phy_devad,
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
            if (BCM_SUCCESS(rv)) {
                phy_wr_data |= (phy_rd_data & ~phy_mask);
                rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
            }
        }
        PORT_UNLOCK(unit);
    }
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_phy_set
 * Description:
 *      General PHY register write
 * Parameters:
 *      unit - Device number
 *      port - Port number or PHY MDIO address (refer BCM_PORT_PHY_NOMAP)
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_reg_addr - PHY internal register address
 *      phy_data - Data to write
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_set(int unit, bcm_gport_t port, uint32 flags,
                          uint32 phy_reg_addr, uint32 phy_data)
{
#ifdef PORTMOD_SUPPORT
    uint16 phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;
    portctrl_pport_t pport=0;


    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));
        if(!(SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port))) {
            return BCM_E_PORT;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "portctrl_phy_set: u=%d p=%d flags=0x%08x "
                         "phy_reg=0x%08x phy_data=0x%08x\n"),
                         unit, port, flags, phy_reg_addr, phy_data));

    if (flags & (BCM_PORT_PHY_I2C_DATA8 | BCM_PORT_PHY_I2C_DATA16 | BCM_PORT_PHY_PVT_DATA)) {
        PORT_LOCK(unit);
        rv = portmod_port_phy_reg_write(unit, pport, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
        return rv;
    }

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & BCM_ESW_PORTCTRL_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~BCM_ESW_PORTCTRL_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = portmod_port_phy_reg_write(unit, pport, 0, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = (uint16) port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            PORT_LOCK(unit);
            phy_id = portmod_port_to_phyaddr_int(unit, port);
            PORT_UNLOCK(unit);
        } else {
            PORT_LOCK(unit);
            phy_id = portmod_port_to_phyaddr(unit, port);
            PORT_UNLOCK(unit);
        }

        phy_wr_data = (uint16) (phy_data & 0xffff);
        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_write(unit, phy_id, phy_devad,
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
        }
        PORT_UNLOCK(unit);
    }
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_phy_drv_name_get
 * Purpose:
 *      Return the name of the PHY driver being used on a port.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 * Returns:
 *      Pointer to static string
 */
int
bcmi_esw_portctrl_phy_drv_name_get(int unit, bcm_gport_t port, char *name,
                                   int len)
{
#ifdef PORTMOD_SUPPORT
    int str_len;
    int rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    rv = PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport);

    if (BCM_FAILURE(rv)) {
        str_len = sal_strlen("invalid port");
        if (str_len <= len) {
            sal_strncpy(name, "invalid port", len);
        }
        return BCM_E_PORT;
    }

    PORT_LOCK(unit);

    rv = portmod_port_phy_drv_name_get(unit, pport, name, len);

    PORT_UNLOCK(unit);

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_portctrl_phy_reset
 * Description:
 *      This function performs the low-level PHY reset and is intended to be
 *      called ONLY from callback function registered with
 *      bcm_port_phy_reset_register. Attempting to call it from any other
 *      place will break lots of things.
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_reset(int unit, bcm_gport_t port)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_reset_set(unit, pport, 
                                phymodResetModeHard /* Hard reset */,
                                0 /* Not used */,
                                phymodResetDirectionInOut /* Toggle reset */);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_ability_advert_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      port_ability - (OUT) Local advertisement in bcm_port_ability_t structure.
 *      ability_mask - (OUT) Local advertisement in bcm_port_abil_t.
 *                           If !NULL, returns mask of BCM_PORT_ABIL_.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_ability_advert_get(int unit, bcm_gport_t port, 
                                     bcm_port_ability_t *port_ability,
                                     bcm_port_abil_t *ability_mask)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;
    portmod_port_ability_t portmod_ability;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    bcm_port_ability_t_init(port_ability);
    sal_memset(&portmod_ability, 0, sizeof(portmod_port_ability_t));

    PORT_LOCK(unit);
    rv = portmod_port_ability_advert_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &portmod_ability);
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        _bcm_esw_portctrl_from_portmod_ability(&portmod_ability, port_ability);

        /* Convert to BCM_PORT_ABIL_ mask if needed */
        if (ability_mask != NULL) {
            rv = soc_port_ability_to_mode(port_ability, ability_mask);
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_UP(unit, port,
                             "Get port ability advert: u=%d p=%d rv=%d\n"),
                unit, port, rv));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_ability_advert_set
 * Purpose:
 *      Set the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      port_ability - Local advertisement in bcm_port_ability_t structure.
 *      ability_mask - Local advertisement in bcm_port_abil_t.
 *                           If !NULL, returns mask of BCM_PORT_ABIL_.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
  * Notes:
 *      This call MAY NOT restart autonegotiation (depending on the phy).
 *      To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */
int
bcmi_esw_portctrl_ability_advert_set(int unit, bcm_gport_t port, 
                                     bcm_port_ability_t *ability_mask,
                                     bcm_port_abil_t port_abil)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;
    bcm_port_ability_t local_ability, temp_ability_mask, temp_ability;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    sal_memset(&local_ability, 0, sizeof(bcm_port_ability_t));
    sal_memset(&temp_ability_mask, 0, sizeof(bcm_port_ability_t));

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN
        (bcmi_esw_portctrl_ability_get(unit, port, &local_ability, NULL));

    /* If ability_mask is NULL then it means the call came through
     * old API bcm_esw_port_advert_set. We need to convert
     * BCM_PORT_ABIL_ mask to bcm_port_ability_t structure */
    if (ability_mask == NULL) {
        SOC_IF_ERROR_RETURN(soc_port_mode_to_ability(port_abil, &temp_ability_mask));
    } else {
        temp_ability_mask = *ability_mask;
    }
    /* temporary store speed_half_duplex ability */
    temp_ability.speed_half_duplex = temp_ability_mask.speed_half_duplex;
    /* Make sure to advertise only abilities supported by the port */
    SOC_IF_ERROR_RETURN(soc_port_ability_mask(&local_ability, &temp_ability_mask));
    /* restore half duplex speed ability, even if some mac don't support it */
    local_ability.speed_half_duplex = temp_ability.speed_half_duplex;

    PORT_LOCK(unit);
    rv = portmod_port_ability_advert_set(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &local_ability);
    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                         "Set port ability advert: u=%d p=%d rv=%d\n"),
              unit, port, rv));

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_UP(unit, port,
                     "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x abl_advert_set\n"
                            "Interface=0x%08x Medium=0x%08x Loopback=0x%08x Flags=0x%08x\n"),
                     local_ability.speed_half_duplex,
                     local_ability.speed_full_duplex,
                     local_ability.pause, local_ability.interface,
                     local_ability.medium, local_ability.loopback,
                     local_ability.flags));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_autoneg_get
 * Purpose:
 *      Get the autonegotiation state of the port
 * Parameters:
 *      unit - Unit #
 *      port - GPORT #
 *      autoneg - (OUT) Boolean value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_autoneg_get(int unit, bcm_gport_t port, int *autoneg)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;
    int phy_lane = -1;
    int phyn = 0, sys_side = 0;
    bcm_port_t local_port = -1;
    phymod_autoneg_control_t an;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    phymod_autoneg_control_t_init(&an);

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_phyn_validate(unit, port,
                                          &local_port, &phyn,
                                          &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    if (local_port == -1) {
        /* Configure outermost PHY (common case) */
        rv = portmod_port_autoneg_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an);
    } else {
        /* Configure PHY specified by GPORT */
        rv = portmod_port_redirect_autoneg_get(unit, pport, phyn,
                                               phy_lane, sys_side, &an);
    }
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        *autoneg = an.enable;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Get port autoneg: u=%d p=%d an=%d rv=%d\n"),
              unit, port, *autoneg, rv));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_autoneg_set
 * Purpose:
 *      Set the autonegotiation state for a given port
 * Parameters:
 *      unit - Unit #
 *      port - GPORT #
 *      autoneg - Boolean value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_autoneg_set(int unit, bcm_port_t port, int autoneg)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;
    int phy_lane = -1;
    int phyn = 0, sys_side = 0;
    bcm_port_t local_port = -1;
    phymod_autoneg_control_t an;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

#ifdef BCM_RCPU_SUPPORT
    if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
#endif /* BCM_RCPU_SUPPORT */

    phymod_autoneg_control_t_init(&an);

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_phyn_validate(unit, port,
                                          &local_port, &phyn,
                                          &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN(portmod_port_autoneg_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));
    an.enable = autoneg;

    PORT_LOCK(unit);
    if (local_port == -1) {
        /* Configure outermost PHY (common case) */
        rv = portmod_port_autoneg_set(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an);
    } else {
        /* Configure PHY specified by GPORT */
        rv = portmod_port_redirect_autoneg_set(unit, pport, phyn,
                                               phy_lane, sys_side, &an);
    }
    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Set port autoneg: u=%d p=%d an=%d rv=%d\n"),
              unit, port, autoneg, rv));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_duplex_get
 * Purpose:
 *      Get the port duplex settings
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      duplex - (OUT) Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_duplex_get(int unit, bcm_gport_t port, int *duplex)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = soc_esw_portctrl_duplex_get(unit, port, duplex);
    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                          "Get port duplex: u=%d p=%d dup=%d rv=%d\n"),
              unit, port, *duplex, rv));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_duplex_set
 * Purpose:
 *      Set the port duplex settings.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      duplex - Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Turns off autonegotiation.  Caller must make sure other forced
 *      parameters (such as speed) are set.
 */
int
bcmi_esw_portctrl_duplex_set(int unit, bcm_port_t port, int duplex)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    pbmp_t      pbm;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

#ifdef BCM_RCPU_SUPPORT
    if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }
#endif /* BCM_RCPU_SUPPORT */

#if 0
    /* Disable auto-negotiation if duplex is forced */
    rv = bcmi_esw_portctrl_autoneg_set(unit, port, FALSE);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                 "Set port autonego failed:%s\n"),
                     bcm_errmsg(rv)));
        return rv;
    }
#endif 

    PORT_LOCK(unit);

    rv = portmod_port_duplex_set(unit, pport, duplex);

    PORT_UNLOCK(unit);                  /* Unlock before link call */

    if (PORTMOD_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
        /* Force link change event */
        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port);
        (void)bcm_esw_link_change(unit, pbm);
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
                         "Set port duplex: u=%d p=%d dup=%d rv=%d\n"),
              unit, port, duplex, rv));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_clear_rx_lss_status_set
 * Purpose:
 *      set the local and remote fault fields of the lss status register.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Port number.
 *      local_fault  - (IN) value of local fault field to be set.
 *      remote_fault - (IN) value of remote fault field to be set.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcmi_esw_portctrl_clear_rx_lss_status_set(int unit, bcm_gport_t port,
                                          int local_fault, int remote_fault)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_clear_rx_lss_status_set(unit, pport, local_fault,
                                              remote_fault);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_mode_setup
 * Purpose:
 *      Set initial operating mode for a port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - Whether to enable or disable
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */
int
bcmi_esw_portctrl_mode_setup(int unit, bcm_gport_t port, int enable)
{
#ifdef PORTMOD_SUPPORT
#if 0
    soc_port_if_t   pif;
#endif
    portmod_port_ability_t  local_port_ability, advert_port_ability;
    portctrl_pport_t pport;
    int rv = BCM_E_NONE;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    if (IS_TDM_PORT(unit, port)) {
        return BCM_E_NONE;
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "_bcm_port_mode_setup: u=%d p=%d\n"), unit, port));

    sal_memset(&local_port_ability,  0, sizeof(bcm_port_ability_t));
    sal_memset(&advert_port_ability, 0, sizeof(bcm_port_ability_t));

    PORT_LOCK(unit);
    rv = portmod_port_ability_local_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &local_port_ability);
    PORT_UNLOCK(unit);
    
    PORTMOD_IF_ERROR_RETURN(rv);

#if 0
    /* If MII supported, enable it, otherwise use TBI */
    if (local_port_ability.interface & (SOC_PA_INTF_MII | SOC_PA_INTF_GMII |
                              SOC_PA_INTF_SGMII | SOC_PA_INTF_XGMII)) {
        if (IS_GE_PORT(unit, port)) {
            pif = SOC_PORT_IF_GMII;
        } else if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) {
            if (local_port_ability.interface & SOC_PA_INTF_XGMII) {
                pif = SOC_PORT_IF_XGMII;
            } else { /*  external GbE phy in xe port mode */
                pif = SOC_PORT_IF_SGMII;
            }
        } else {
            pif = SOC_PORT_IF_MII;
        }
    } else if (local_port_ability.interface & SOC_PA_INTF_CGMII) {
        pif = SOC_PORT_IF_CGMII;
    } else {
        pif = SOC_PORT_IF_TBI;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_interface_set(unit, port, pif)); 
    _bcm_esw_portctrl_interface_config_set(unit, port, pport, config, 0);
#endif

    if (IS_ST_PORT(unit, port)) {

        /* Since stacking port doesn't support flow control,
         * make sure that PHY is not advertising flow control capabilities.
         */

        PORT_LOCK(unit);
        rv = portmod_port_ability_advert_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &advert_port_ability);
        if (PORTMOD_SUCCESS(rv)) {
            advert_port_ability.pause &= ~(SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM);
            rv = portmod_port_ability_advert_set(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY,
                                                 &advert_port_ability);
        }
        PORT_UNLOCK(unit);
    }
    
    PORTMOD_IF_ERROR_RETURN(rv);

    if (!SOC_WARM_BOOT(unit) &&
        !(SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, all), port)) ) {
#ifdef BCM_RCPU_SUPPORT
        if (SOC_IS_RCPU_ONLY(unit) && IS_RCPU_PORT(unit, port)) {
            /* Do not enable/disable rcpu port. */
            return BCM_E_NONE;
        }
#endif /* BCM_RCPU_SUPPORT */
        PORT_LOCK(unit);
        rv = _bcm_esw_portctrl_enable_set(unit, port, pport,
                                          PORTMOD_PORT_ENABLE_MAC, enable); 
        PORT_UNLOCK(unit);
        BCM_IF_ERROR_RETURN(rv);
    }

    return BCM_E_NONE;
#else /* PORTMOD_SUPPORT */
        return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_clear_rx_lss_status_get
 * Purpose:
 *      get the local and remote fault fields of the lss status register.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      port         - (IN) Port number.
 *      local_fault  - (OUT) value of local fault field to get.
 *      remote_fault - (OUT) value of remote fault field to get.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcmi_esw_portctrl_clear_rx_lss_status_get(int unit, bcm_gport_t port,
                                          int *local_fault,
                                          int *remote_fault)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_clear_rx_lss_status_get(unit, pport, local_fault,
                                              remote_fault);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      _bcm_esw_portctrl_detach
 * Purpose:
 *      Main part of bcm_port_detach
 */

#ifdef PORTMOD_SUPPORT
STATIC int
_bcm_esw_portctrl_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    soc_port_t port;
    portctrl_pport_t pport;
    soc_persist_t *sop = SOC_PERSIST(unit); 
    int rv;
    int flags = 0;
    portmod_port_add_info_t port_add_info;
    int num_lanes;
    int lane;

    PORTMOD_PORT_ENABLE_PHY_SET(flags);
    PORTMOD_PORT_ENABLE_MAC_SET(flags);

    SOC_PBMP_CLEAR(*detached);

    SOC_PBMP_ITER(pbmp, port) {

        BCM_IF_ERROR_RETURN
            (bcm_esw_port_stp_set(unit, port, BCM_STG_STP_DISABLE));
        rv = bcmi_esw_portctrl_mode_setup(unit, port, FALSE);
        BCM_IF_ERROR_RETURN(rv);

        BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_portctrl_enable_set(unit, port, pport, flags, 0));
        PORTMOD_IF_ERROR_RETURN(portmod_port_remove(unit, port));

        PORTMOD_IF_ERROR_RETURN(portmod_port_add_info_t_init(unit, &port_add_info));
        port_add_info.interface_config.interface = SOC_PORT_IF_NULL;
        PORTMOD_IF_ERROR_RETURN(portmod_port_add(unit, port, &port_add_info));

        SOC_PBMP_PORT_ADD(*detached, port);

        if (SOC_PBMP_MEMBER(sop->lc_pbm_fc, port)) {
            /* Bring down the internal PHY */
            rv = bcm_esw_port_update(unit, port, FALSE);
            BCM_IF_ERROR_RETURN(rv);
            SOC_PBMP_PORT_REMOVE(sop->lc_pbm_fc, port);
        }

        /*
         * Detach xphy lanes from the port.
         */
        num_lanes = SOC_INFO(unit).port_num_lanes[port];
        /*
         * Need to change to 12 because not pm12x10 mode are consecutive lanes.
         * Only 442 is, other like 343 and 244 skip 1 and 2 lanes in the middle.
         */
        if (num_lanes == 10) {
            num_lanes=12;
        }
        for (lane = 0; lane < num_lanes; lane++) {
            portmod_xphy_lane_detach(unit, SOC_INFO(unit).port_l2p_mapping[port]+lane, 1);
        }
    }

    return BCM_E_NONE;
}
#endif /* PORTMOD_SUPPORT */

/*
 * Function:
 *      bcmi_esw_portctrl_detach
 * Purpose:
 *      Detach a port.  Set phy driver to no connection.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      pbmp - Bitmap of ports to detach.
 *      detached (OUT) - Bitmap of ports successfully detached.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      If a port to be detached does not appear in detached, its
 *      state is not defined.
 */

int
bcmi_esw_portctrl_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
#ifdef PORTMOD_SUPPORT
    char pfmtp[SOC_PBMP_FMT_LEN],
         pfmtd[SOC_PBMP_FMT_LEN];
    int rv = BCM_E_NONE;

    PORTCTRL_INIT_CHECK(unit);

    PORT_LOCK(unit);

    rv = _bcm_esw_portctrl_detach(unit, pbmp, detached);

    PORT_UNLOCK(unit);

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
             "bcm_port_detach: u=%d pbmp=%s det=%s rv=%d\n"),
             unit,
             SOC_PBMP_FMT(pbmp, pfmtp),
             SOC_PBMP_FMT(*detached, pfmtd),
             rv));

    return rv;
#else /* PORTMOD_SUPPORT */
        return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_frame_max_get
 * Purpose:
 *      Get the maximum receive frame size for the port.
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      size   - (OUT) Maximum frame size in bytes.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Depending on chip or port type the actual maximum receive frame size
 *      might be slightly higher.
 *
 *      For GE ports that use 2 separate MACs (one for GE and another one for
 *      10/100 modes) the function returns the maximum rx frame size set for
 *      the current mode.
 */
int
bcmi_esw_portctrl_frame_max_get(int unit, bcm_gport_t port, int *size)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = soc_esw_portctrl_frame_max_get(unit, port, size);
    PORT_UNLOCK(unit);

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_ifg_set
 * Purpose:
 *      Set Inter Frame Gap 
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      speed  - Speed
 *      duplex  - Duplex Setting
 *      ifg  -   InterFrame Gap
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      
 *      
 */
int
bcmi_esw_portctrl_ifg_set(int unit, bcm_gport_t port, int speed, 
                          bcm_port_duplex_t duplex, int ifg)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit); 
    rv = soc_esw_portctrl_ifg_set(unit, port, speed, duplex, ifg);
    PORT_UNLOCK(unit);

    return rv;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_frame_max_set
 * Purpose:
 *      Set the maximum receive frame size for the port.

 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      mac    - (IN) Station MAC address used for pause frames.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_frame_max_set(int unit, bcm_gport_t port, int size)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = soc_esw_portctrl_frame_max_set(unit, port, size);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_ifg_get
 * Purpose:
 *      Set Inter Frame Gap 
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      speed  - Speed
 *      duplex  - Duplex Setting
 *      ifg  -   InterFrame Gap
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      
 *      
 */
int
bcmi_esw_portctrl_ifg_get(int unit, bcm_gport_t port, int speed, 
                          bcm_port_duplex_t duplex, int *ifg)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;
  
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = soc_esw_portctrl_ifg_get(unit, port, speed, duplex, ifg);
    PORT_UNLOCK(unit);

    return rv;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_mac_rx_control
 * Purpose:
 *      Set Inter Frame Gap 
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      optype  - operation type
 *      enable  - enable/disable
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      
 *      
 */

int
bcmi_esw_portctrl_mac_rx_control(int unit, bcm_port_t port, uint8 optype, int *enable)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;

    PORT_LOCK(unit);
    if(optype) {
        rv = portmod_port_rx_mac_enable_get(unit, port, enable); 
    } else {
        rv = portmod_port_rx_mac_enable_set(unit, port, *enable);
    }
    PORT_UNLOCK(unit);
    return(rv);
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_mac_up
 * Purpose:
 *      Bring up MAC 
 * Parameters:
 *      unit  - (IN) StrataSwitch unit number.
 *      nport - (IN) Number of elements in array port
 *      port  - (IN) Array of logical port.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 */
int
bcmi_esw_portctrl_mac_up(int unit, int nport, bcm_port_t *port)
{
#ifdef PORTMOD_SUPPORT
    int i;
    bcm_port_t lport;
    portctrl_pport_t pport;

    for (i = 0; i < nport; i++) {
        lport = port[i];

        BCM_IF_ERROR_RETURN
            (PORTCTRL_PORT_RESOLVE(unit, lport, &lport, &pport));

        /* Remove MAC reset */
        BCM_IF_ERROR_RETURN
            (portmod_port_mac_reset_set(unit, pport, 0));

        /* Disable port flush state */
        BCM_IF_ERROR_RETURN
            (portmod_port_discard_set(unit, pport, 0) );

        /* Enable MAC TX */
        BCM_IF_ERROR_RETURN
            (portmod_port_rx_mac_enable_set(unit, pport, 1));

        /* Enable MAC RX */
        BCM_IF_ERROR_RETURN
            (portmod_port_tx_mac_enable_set(unit, pport, 1));
    }
    return BCM_E_NONE;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_mac_rx_down
 * Purpose:
 *      Disable MAC receive 
 * Parameters:
 *      unit  - (IN) StrataSwitch unit number.
 *      nport - (IN) Number of elements in array port
 *      port  - (IN) Array of logical port.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 */
int
bcmi_esw_portctrl_mac_rx_down(int unit, int nport, bcm_port_t *port)
{
#ifdef PORTMOD_SUPPORT
    int i;
    bcm_port_t lport;
    portctrl_pport_t pport;

    for (i = 0; i < nport; i++) {
        lport = port[i];

        BCM_IF_ERROR_RETURN
            (PORTCTRL_PORT_RESOLVE(unit, lport, &lport, &pport));

        /* Disable MAC RX */
        BCM_IF_ERROR_RETURN
            (portmod_port_rx_mac_enable_set(unit, pport, 0));

        /* Put port in flush state */
        BCM_IF_ERROR_RETURN
            (portmod_port_discard_set(unit, pport, 1) );
    }
    return BCM_E_NONE;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_mac_tx_down
 * Purpose:
 *      Poll until MAC TX FIFO is empty
 *      Disable MAC transmit 
 * Parameters:
 *      unit  - (IN) StrataSwitch unit number.
 *      nport - (IN) Number of elements in array port
 *      port  - (IN) Array of logical port.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 */
int
bcmi_esw_portctrl_mac_tx_down(int unit, int nport, bcm_port_t *port)
{
#ifdef PORTMOD_SUPPORT
    int i;
    bcm_port_t lport;
    portctrl_pport_t pport;

    for (i = 0; i < nport; i++) {
        lport = port[i];

        BCM_IF_ERROR_RETURN
            (PORTCTRL_PORT_RESOLVE(unit, lport, &lport, &pport));

        /* Drain MAC TX FIFO cells */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_portctrl_drain_cells(unit, pport, lport));

        /* MAC TX disable */
        BCM_IF_ERROR_RETURN
            (portmod_port_tx_mac_enable_set(unit, pport, 0));

        /* Hold MAC in reset state */
        BCM_IF_ERROR_RETURN
            (portmod_port_mac_reset_set(unit, pport, 1));

    }
    return BCM_E_NONE;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_encap_get
 * Purpose:
 *      Get the port encapsulation mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      mode (OUT) - One of BCM_PORT_ENCAP_xxx (see port.h)
 * Returns:
 *      BCM_E_XXX
 */

int
bcmi_esw_portctrl_encap_get(int unit, bcm_gport_t port, int *mode)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE, flags=0, valid = 0;
    portctrl_pport_t pport;
    portmod_port_interface_config_t port_if_cfg;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN(portmod_port_is_valid(unit, pport, &valid));
    if (!valid) {
        return BCM_E_PORT;
    }

    if (IS_GE_PORT(unit, port) && IS_ST_PORT(unit, port)) {
        if (soc_feature(unit, soc_feature_embedded_higig)) {
            *mode = BCM_PORT_ENCAP_IEEE;
        } else {
            portmod_encap_t val;

            PORT_LOCK(unit);
            rv = portmod_port_encap_get(unit, pport, &flags, &val);
            PORT_UNLOCK(unit);

            if (PORTMOD_SUCCESS(rv)) {
                *mode = val ? BCM_PORT_ENCAP_HIGIG2 : BCM_PORT_ENCAP_IEEE;
            } else {
                return (BCM_E_CONFIG);
            }
        }
    } else {
        portmod_port_interface_config_t_init(unit, &port_if_cfg);

        PORT_LOCK(unit);
        rv = portmod_port_interface_config_get(unit, pport, &port_if_cfg, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);
        PORT_UNLOCK(unit);

        if (PORTMOD_SUCCESS(rv)) {
            *mode = port_if_cfg.encap_mode;
        }
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
              "Port encap get: u=%d p=%d mode=%d rv=%d\n"),
              unit, port, *mode, rv));

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}
 
#ifdef PORTMOD_SUPPORT
/*
 * Function:
 *      _bcm_esw_portctrl_interface_cfg_set
 * Purpose:
 *      This function is called by encap set functions to set encap mode,
 *      interface type and speed
 * Parameters:
 *      unit        - (IN) Unit number
 *      port        - (IN) Logical BCM Port number (non-GPORT).
 *      pport       - (IN) Portmod Port number
 *      field       - (IN) Field of portmod_port_interface_config_t to modify
 *                         Only encap_mode, speed and interface can be 
 *                         modified
 *      field_val   - Value of the Portmod field to program
 * Returns:
 *      BCM_E_XXX
 */
STATIC
int _bcm_esw_portctrl_interface_cfg_set(int unit, bcm_port_t port,
                                        portctrl_pport_t pport,
                                        int field, void *field_val)
{
    portmod_port_interface_config_t port_if_cfg;
    soc_port_if_t                   default_interface;
    char *str;
    int rv = BCM_E_NONE;

    portmod_port_interface_config_t_init(unit, &port_if_cfg);

    PORT_LOCK(unit);

    rv = portmod_port_interface_config_get(unit, pport, &port_if_cfg, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);

    if ((port_if_cfg.encap_mode == SOC_ENCAP_HIGIG2) ||
        (port_if_cfg.encap_mode == SOC_ENCAP_HIGIG)) {
        PHYMOD_INTF_MODES_HIGIG_SET(&port_if_cfg);
    } else {
        PHYMOD_INTF_MODES_HIGIG_CLR(&port_if_cfg);
    }

    if (PORTMOD_SUCCESS(rv)) {

        /* Set value of appropriate field */
        switch (field) {
            case BCM_ESW_PORTCTRL_CFG_ENCAP_MODE:
                if (port_if_cfg.encap_mode != *((portmod_encap_t *)field_val)) {
                    port_if_cfg.encap_mode = *((portmod_encap_t *)field_val);

                    /* need to update the port mode since encap is changed */
                    if ((port_if_cfg.encap_mode == SOC_ENCAP_HIGIG2) ||
                        (port_if_cfg.encap_mode == SOC_ENCAP_HIGIG)) {
                        PHYMOD_INTF_MODES_HIGIG_SET(&port_if_cfg);
                    } else {
                        PHYMOD_INTF_MODES_HIGIG_CLR(&port_if_cfg);
                    }

                    /* When changing the encap, the existing interface type of the
                     * port may not be valid anymore. Need to retrieve the default
                     * interface type based on the new encap mode and update 
                     * port_if_cfg.
                     */
                    default_interface = SOC_PORT_IF_NULL;
                    SOC_IF_ERROR_RETURN(
                            portmod_port_default_interface_get
                                (unit, port, &port_if_cfg, &default_interface));
                    if (default_interface != SOC_PORT_IF_NULL) {
                        port_if_cfg.interface = default_interface;
                    } else {
                        SOC_IF_ERROR_RETURN(
                                portmod_common_default_interface_get(&port_if_cfg));
                    } 
                }
                break;

            case BCM_ESW_PORTCTRL_CFG_INTERFACE:
                port_if_cfg.interface = *((soc_port_if_t *)field_val);
                break;

            case BCM_ESW_PORTCTRL_CFG_SPEED:
                port_if_cfg.speed = *((int *)field_val);
                break;

            default:
                PORT_UNLOCK(unit);
                return BCM_E_PARAM;
        }

        
        if (port_if_cfg.speed == 0) {
            port_if_cfg.speed = SOC_CONTROL(unit)->info.port_speed_max[port];
        }
        
        rv = _bcm_esw_portctrl_interface_config_set(unit, port, pport,
                                                    &port_if_cfg, 0);
        PORT_UNLOCK(unit);

        if (PORTMOD_FAILURE(rv)) {
            str = "set";
        }
    } else {
        PORT_UNLOCK(unit);
        str = "get";
    }

    if (PORTMOD_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit, 
                    "Interface_%s failed:err=%d: %s\n"), str, rv,
                    bcm_errmsg(rv)));

        rv = BCM_E_CONFIG; /* Returning BCM_ error code */
    }

    return rv;
}
#endif /* PORTMOD_SUPPORT */

/*
 * Function:
 *      bcmi_esw_portctrl_encap_xport_set
 * Purpose:
 *      Convert Ether port to Higig port, or reverse
 * Parameters:
 *      unit   - (IN) Unit number.
 *      port   - (IN) Port number.
 *      mode   - (IN) Encapsulation mode
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */

int
bcmi_esw_portctrl_encap_xport_set(int unit, bcm_gport_t port, int mode)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    bcm_stg_t        stg;
    int              to_higig;
    soc_field_t      port_type_field;
    soc_reg_t        egr_port_reg;
    soc_port_ability_t ability;
    soc_port_mode_t  non_ieee_speed;
    int              force_speed, port_type, higig_type;
    phymod_autoneg_control_t an_info;
    bcm_port_abil_t ability_mask=0;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    to_higig = (mode != BCM_PORT_ENCAP_IEEE);

    port_type_field = PORT_TYPEf;

    rv = _bcm_esw_portctrl_interface_cfg_set(unit, port, pport,
                                             BCM_ESW_PORTCTRL_CFG_ENCAP_MODE,
                                             (void *)&mode);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
   

#if 0
    if (to_higig) {
        soc_port_if_t port_intf;

        port_intf = SOC_PORT_IF_XGMII;
        rv = _bcm_esw_portctrl_interface_cfg_set(unit, port, pport,
                                                 BCM_ESW_PORTCTRL_CFG_INTERFACE,
                                                 (void *)&port_intf);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
#endif 

    /* Specific for GH on avoiding port speed change */
    if (!soc_feature(unit, soc_feature_hg_no_speed_change)) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_ability_local_get(unit, port,
                            &ability));

        non_ieee_speed = ability.speed_full_duplex &
                ~(SOC_PA_SPEED_100GB | SOC_PA_SPEED_40GB | SOC_PA_SPEED_10GB |
                  SOC_PA_SPEED_2500MB | SOC_PA_SPEED_1000MB |
                  SOC_PA_SPEED_100MB | SOC_PA_SPEED_10MB);

        BCM_IF_ERROR_RETURN(bcm_esw_port_ability_advert_get(unit, port,
                            &ability));
        SOC_IF_ERROR_RETURN(soc_esw_portctrl_speed_get(unit, port,
                                                       &force_speed));
        if (to_higig) {
            ability.speed_full_duplex |= non_ieee_speed;
            ability.pause &= ~(SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM);
            force_speed = force_speed == 0 ?
                          SOC_CONTROL(unit)->info.port_speed_max[port] :
                          force_speed;
            ability.speed_full_duplex |= SOC_PA_SPEED(force_speed);
        } else {
            ability.speed_full_duplex &= ~non_ieee_speed;
            force_speed = SOC_PORTCTRL_HG2_TO_IEEE_BW_GET(force_speed);
        }

        BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_ability_advert_set(unit, port,
                                                                 &ability,
                                                                 ability_mask));

        phymod_autoneg_control_t_init(&an_info);
        
        PORT_LOCK(unit);
        BCM_IF_ERROR_RETURN(portmod_port_autoneg_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an_info));
        PORT_UNLOCK(unit);

        /* Some mac driver re-init phy while executing MAC_ENCAP_SET, in that
         * case autoneg is probably always true here
         */
        if (an_info.enable) {
            SOC_IF_ERROR_RETURN(bcm_esw_port_autoneg_set(unit, port, TRUE));
        } else {
            rv = _bcm_esw_portctrl_interface_cfg_set(unit, port, pport,
                                                     BCM_ESW_PORTCTRL_CFG_SPEED,
                                                     (void *)&force_speed);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }
    }

    /* Now we propagate the changes */
    port_type = to_higig ? 1 : 0;

    if (port_type == 1) {
        if (mode == BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET) {
            higig_type = BCM_PORT_HG_TYPE_HGOE_TRANSPORT;
        } else if (mode == BCM_PORT_ENCAP_HIGIG) {
            higig_type = BCM_PORT_HG_TYPE_HIGIGPLUS;
        } else {
            higig_type = BCM_PORT_HG_TYPE_HIGIG2_NORMAL;
        }
    } else {
        higig_type =  0;
    }

    if (soc_mem_field_valid(unit, PORT_TABm, port_type_field)) {
        SOC_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit, port,
                            _BCM_CPU_TABS_NONE, port_type_field, port_type));
    }
    if (SOC_MEM_IS_VALID(unit, EGR_PORTm)) {
        if (soc_mem_field_valid(unit, EGR_PORTm, port_type_field)) {
            SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_PORTm, port,
                                port_type_field, port_type));
        }
        if(soc_mem_field_valid(unit, EGR_PORTm, HG_TYPEf)) {
            SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_PORTm, port,
                                HG_TYPEf, higig_type));
        }
    } else {
        if (SOC_REG_IS_VALID(unit, EGR_PORT_64r)) {
            egr_port_reg = EGR_PORT_64r;
        } else {
            egr_port_reg = EGR_PORTr;
        }
        SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit, egr_port_reg, port,
                            port_type_field, port_type));
    }

    if (!IS_CPU_PORT(unit, port) && !IS_LB_PORT(unit, port) && !IS_RDB_PORT(unit, port)) {
        if (SOC_MEM_IS_VALID(unit, EGR_ING_PORTm)) {
            SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_ING_PORTm,
                                port, PORT_TYPEf, port_type));
        }
        if(SOC_MEM_FIELD_VALID(unit, EGR_ING_PORTm, HG_TYPEf)) {
            SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_ING_PORTm,
                                port, HG_TYPEf, higig_type));
        }
    }

    
    if (SOC_MEM_IS_VALID(unit, ICONTROL_OPCODE_BITMAPm)) {
        icontrol_opcode_bitmap_entry_t entry;
        soc_pbmp_t pbmp;

        SOC_IF_ERROR_RETURN(READ_ICONTROL_OPCODE_BITMAPm(unit, MEM_BLOCK_ANY,
                            port, &entry));
        SOC_PBMP_CLEAR(pbmp);
        if (to_higig) {
            SOC_PBMP_PORT_SET(pbmp, CMIC_PORT(unit));
        }
        soc_mem_pbmp_field_set(unit, ICONTROL_OPCODE_BITMAPm, &entry, BITMAPf,
                               &pbmp);
        SOC_IF_ERROR_RETURN(WRITE_ICONTROL_OPCODE_BITMAPm(unit, MEM_BLOCK_ANY,
                                                          port, &entry));
    } else {

        /* Set HG ingress CPU Opcode map to the CPU */
        /* int pbm_len; */
        uint32 cpu_pbm = 0;

        if (to_higig) {
            SOC_IF_ERROR_RETURN(soc_xgs3_port_to_higig_bitmap(unit,
                                CMIC_PORT(unit), &cpu_pbm));
        } /* else, cpu_pbm = 0 */

        SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit,
                            ICONTROL_OPCODE_BITMAPr, port, BITMAPf, cpu_pbm));
    }

    if (to_higig) {
        /* Embedded Higig ports should be configured and marked as ST ports */
        BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_pause_set(unit, port, 0, 0));
        /* HG ports to forwarding */
        BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port,
                                                 BCM_STG_STP_FORWARD));
    } else { 
        BCM_IF_ERROR_RETURN(bcmi_esw_portctrl_pause_set(unit, port, 1, 1));
    }
    /* Clear mirror enable settings */
    /* coverity[stack_use_callee] */
    /* coverity[stack_use_overflow] */
    BCM_IF_ERROR_RETURN(bcm_esw_mirror_port_set(unit, port, -1, -1, 0));

    /* Set untagged state in default VLAN properly */
    BCM_IF_ERROR_RETURN(_bcm_esw_vlan_untag_update(unit, port, to_higig));

    /* Resolve STG 0 */
    BCM_IF_ERROR_RETURN(bcm_esw_stg_default_get(unit, &stg));
    BCM_IF_ERROR_RETURN(bcm_esw_stg_stp_set(unit, 0, port,
                        to_higig ? BCM_STG_STP_FORWARD : BCM_STG_STP_DISABLE));

#ifdef BCM_TRX_SUPPORT
    /* Reset the vlan default action */
    if (soc_feature(unit, soc_feature_vlan_action)) {
        bcm_vlan_action_set_t action;

        BCM_IF_ERROR_RETURN(_bcm_trx_vlan_port_egress_default_action_get(unit,
                            port, &action));
        /* Backward compatible defaults */
        if (to_higig) {
            action.ot_outer = bcmVlanActionDelete;
            action.dt_outer = bcmVlanActionDelete;
        } else {
            action.ot_outer = bcmVlanActionNone;
            action.dt_outer = bcmVlanActionNone;
        }
        BCM_IF_ERROR_RETURN(_bcm_trx_vlan_port_egress_default_action_set(unit,
                            port, &action));
    }
#endif

#ifdef INCLUDE_L3
    if (soc_feature(unit, soc_feature_ip_mcast)) {
        BCM_IF_ERROR_RETURN(bcm_esw_ipmc_egress_port_set(
                unit, port, 
                to_higig ? _soc_mac_all_ones :_soc_mac_all_zeroes, 
                0, 0, 0));
    }
#endif /* INCLUDE_L3 */

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_encap_higig_lite_set
 * Purpose:
 *      Helper function to force a port into HIGIG2_LITE mode
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) port number
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_encap_higig_lite_set(int unit, bcm_gport_t port)
{
#ifdef PORTMOD_SUPPORT
    portctrl_pport_t  pport;
    portmod_port_ability_t  ability;
    phymod_autoneg_control_t an_info;
    int rv;

    PORTCTRL_INIT_CHECK(unit);

    sal_memset(&ability, 0, sizeof(soc_port_ability_t));
    sal_memset(&an_info, 0, sizeof(phymod_autoneg_control_t));

    if (soc_feature(unit, soc_feature_hg_no_speed_change)) {
        /* HG-Lite process without 2.5G speed setting can calls to
         * bcmi_esw_portctrl_encap_xport_set() directly.
         */
        return bcmi_esw_portctrl_encap_xport_set(unit, port,
                                                 BCM_PORT_ENCAP_HIGIG2_LITE);
    }

    if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {

        BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port,
                            &pport));

        /* Restrict the speed to <= 2.5G and set the encap to HG2 */
        BCM_IF_ERROR_RETURN(portmod_port_ability_local_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY,
                            &ability));

        ability.speed_full_duplex &= ~(SOC_PA_SPEED_10GB |
                SOC_PA_SPEED_12GB | SOC_PA_SPEED_13GB | SOC_PA_SPEED_15GB |
                SOC_PA_SPEED_16GB | SOC_PA_SPEED_20GB | SOC_PA_SPEED_21GB |
                SOC_PA_SPEED_25GB | SOC_PA_SPEED_30GB | SOC_PA_SPEED_40GB);

        BCM_IF_ERROR_RETURN(portmod_port_ability_advert_set(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY,
                            &ability));
        BCM_IF_ERROR_RETURN(portmod_port_autoneg_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY,
                            &an_info));

        if (!an_info.enable) {
            int speed;

            speed = 2500;
            rv = _bcm_esw_portctrl_interface_cfg_set(unit, port, pport,
                                                     BCM_ESW_PORTCTRL_CFG_SPEED,
                                                     (void *)&speed);

            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }

        return bcmi_esw_portctrl_encap_xport_set(unit, port,
                                                 BCM_PORT_ENCAP_HIGIG2);
    } else if (IS_ST_PORT(unit, port) || IS_E_PORT(unit, port)) {

        return _bcm_port_encap_stport_set(unit, port, BCM_PORT_ENCAP_HIGIG2);
    }

    return BCM_E_CONFIG;
#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_higig_mode_set
 * Purpose:
 *      Set port Higig mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      higig_mode - Higig mode
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_higig_mode_set(int unit, bcm_gport_t port, int higig_mode)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port,
                                              &pport));

    PORT_LOCK(unit);
    rv = portmod_port_higig_mode_set(unit, pport, higig_mode);
    PORT_UNLOCK(unit);

    return rv;
#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}
/*
 * Function:
 *      bcmi_esw_portctrl_higig2_mode_set
 * Purpose:
 *      Set port Higig2 mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      higig2_mode - Higig2 mode
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_higig2_mode_set(int unit, bcm_gport_t port, int higig2_mode)
{
#ifdef PORTMOD_SUPPORT
    soc_reg_t egr_port_reg;
    soc_mem_t egr_port_mem;
    int rv;
    portctrl_pport_t pport;
    int port_hg2_mode = higig2_mode;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port,
                                              &pport));

    BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit, port,
                        _BCM_CPU_TABS_NONE, HIGIG2f, higig2_mode));

    if (SOC_REG_IS_VALID(unit, EGR_PORT_64r)) {
        egr_port_reg = EGR_PORT_64r;
    } else {
        egr_port_reg = EGR_PORTr;
    }

    if (SOC_REG_FIELD_VALID(unit, egr_port_reg, HIGIG2f)) {
        BCM_IF_ERROR_RETURN(soc_reg_field32_modify(unit, egr_port_reg, port,
                                                   HIGIG2f, higig2_mode));
    } else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, HIGIG2f)) {
        BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_PORTm, port,
                                                   HIGIG2f, higig2_mode));
    }

    PORT_LOCK(unit);
    rv = portmod_port_higig2_mode_set(unit, pport, port_hg2_mode);
    PORT_UNLOCK(unit);

    if (PORTMOD_FAILURE(rv)) {
        return rv;
    }

    egr_port_mem = EGR_ING_PORTm;
    if (SOC_MEM_FIELD_VALID(unit, egr_port_mem, HIGIG2f)) {
        if (IS_CPU_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, egr_port_mem,
                                SOC_INFO(unit).cpu_hg_index, HIGIG2f,
                                higig2_mode));
        } else {
            BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, egr_port_mem,
                                                       port, HIGIG2f,
                                                       higig2_mode));
        }
    }

    return BCM_E_NONE;
#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

#ifdef PORTMOD_SUPPORT
/*
 * Function:
 *      bcmi_esw_portctrl_encap_set_execute
 * Purpose:
 *      Set the port encapsulation mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      mode - One of BCM_PORT_ENCAP_xxx (see port.h)
 *      force - TRUE  : force encap operation(set xport_swap = TRUE)
 *                      regardless of IS_HG_PORT and mode value.
 *
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_encap_set_execute(int unit, bcm_gport_t port, int mode, int force)
{
    int         rv, xport_swap = FALSE;
    uint64      val64;
    portctrl_pport_t pport;
    int to_higig;
    int size;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_UP(unit, port,
             "bcm_esw_port_encap_set: u=%d p=%d mode=%d\n"),
              unit, port, mode));

    _bcm_esw_port_mirror_lock(unit);

    if (force) {
        xport_swap = TRUE;
    }

    if ((IS_HG_PORT(unit,port) && (mode == BCM_PORT_ENCAP_IEEE)) ||
        ((IS_XE_PORT(unit, port) || IS_CE_PORT(unit, port) ||
          IS_C_PORT(unit, port)) &&
         (mode != BCM_PORT_ENCAP_IEEE))) {
        if (soc_feature(unit, soc_feature_xport_convertible)) {
            xport_swap = TRUE;
        } else {
            /* Ether <=> Higig not allowed on all systems */
            _bcm_esw_port_mirror_unlock(unit);
            return BCM_E_UNAVAIL;
        }
    }

    if ((mode == BCM_PORT_ENCAP_HIGIG)
        && soc_feature(unit, soc_feature_no_higig_plus)) {
        _bcm_esw_port_mirror_unlock(unit);
        return BCM_E_UNAVAIL;
    }

    if (xport_swap) {
        COUNTER_LOCK(unit);
        if ((BCM_PORT_ENCAP_HIGIG2_LITE == mode)) {
            rv = bcmi_esw_portctrl_encap_higig_lite_set(unit, port);
        } else {
            rv = bcmi_esw_portctrl_encap_xport_set(unit, port, mode);
        }
        
        COUNTER_UNLOCK(unit);
        if (PORTMOD_FAILURE(rv)) {
            _bcm_esw_port_mirror_unlock(unit);
            return rv;
        }
    } else if (IS_HG_PORT(unit, port)) {

        rv = _bcm_esw_portctrl_interface_cfg_set(unit, port, pport,
                                                BCM_ESW_PORTCTRL_CFG_ENCAP_MODE,
                                                (void *)&mode);
            if (BCM_FAILURE(rv)) {
                _bcm_esw_port_mirror_unlock(unit);
                return rv;
            }
    } else if (IS_GE_PORT(unit, port) && IS_ST_PORT(unit, port)) {
        if (mode == BCM_PORT_ENCAP_IEEE) {
            bcm_port_encap_config_t encap_config;

            rv = bcm_esw_port_encap_config_get(unit, port,
                                               &encap_config);
            if (BCM_SUCCESS(rv)) {
                if ((BCM_PORT_ENCAP_HIGIG2_L2 == encap_config.encap) ||
                    (BCM_PORT_ENCAP_HIGIG2_IP_GRE == encap_config.encap) ||
                    (BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET == encap_config.encap) ||
                    (BCM_PORT_ENCAP_HIGIG2_LITE == encap_config.encap)) {
                    rv = _bcm_port_encap_stport_set(unit, port,
                                                    BCM_PORT_ENCAP_IEEE);
                } else {
                    rv = BCM_E_UNAVAIL;
                }
            }
        } else {
            if ((mode == BCM_PORT_ENCAP_HIGIG2) ||
                (mode == BCM_PORT_ENCAP_HIGIG2_LITE)) {
                rv = BCM_E_NONE;
            } else {
                rv = BCM_E_UNAVAIL;
            }
        }
    } else if (IS_CE_PORT(unit, port) || IS_C_PORT(unit, port)) {
         if ((mode == BCM_PORT_ENCAP_HIGIG2) ||
             (mode == BCM_PORT_ENCAP_HIGIG) ||
             (mode == BCM_PORT_ENCAP_IEEE)) {
             rv = _bcm_port_encap_stport_set(unit, port, mode);
         } else {
             rv = BCM_E_UNAVAIL;
         }
    } else if (mode == BCM_PORT_ENCAP_IEEE) {
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_UNAVAIL;
    }

#ifdef BCM_GXPORT_SUPPORT
    if (IS_GX_PORT(unit, port) || IS_XG_PORT(unit,port) || 
        IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port) ||
        IS_CE_PORT(unit, port) || IS_C_PORT(unit, port)) {
        int hg2 = FALSE;

        if (mode == BCM_PORT_ENCAP_HIGIG2 ||
            mode == BCM_PORT_ENCAP_HIGIG2_LITE) {
            hg2 = TRUE;
        }

        if (BCM_SUCCESS(rv)) {
            rv = bcmi_esw_portctrl_higig2_mode_set(unit, port, hg2);
        }

        if (BCM_SUCCESS(rv)) {
            /* No need for locking; _bcm_esw_port_mirror_lock does it */
            uint32 port_hg_mode = (mode == BCM_PORT_ENCAP_IEEE) ? 0 : 1;

            rv = portmod_port_higig_mode_set(unit, pport, port_hg_mode);
        }

        if (PORTMOD_SUCCESS(rv)) {
            if (SOC_MEM_IS_VALID(unit, EGR_PORTm)) {
                rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                                            PORT_TYPEf,
                                            mode == BCM_PORT_ENCAP_IEEE ?
                                            0 : 1);
            }
        }
    }
#endif /* BCM_GXPORT_SUPPORT */

    if (soc_feature(unit, soc_feature_embedded_higig)) {
        /* Clear embedded Higig regs, if present */
        soc_reg_t   ehg_tx_reg = EHG_TX_CONTROLr;
        soc_reg_t   ehg_rx_reg = EHG_RX_CONTROLr;

        if (BCM_SUCCESS(rv) && SOC_REG_IS_VALID(unit, ehg_tx_reg)) {
            rv = soc_reg32_set(unit, ehg_tx_reg, port, 0, 0);
        }
        if (BCM_SUCCESS(rv) && SOC_REG_IS_VALID(unit, ehg_rx_reg)) {
            rv = soc_reg32_set(unit, ehg_rx_reg, port, 0, 0);
        }
    }

    if (BCM_SUCCESS(rv) && soc_feature(unit, soc_feature_higig_over_ethernet)) {
        /* Disable HGoE if needed*/

        uint32 tab_port_type;
        uint32 tab_higig_type;

        tab_port_type =  (mode == BCM_PORT_ENCAP_IEEE) ? 0 : 1;

        if (tab_port_type == 1) {
            if (mode == BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET) {
                tab_higig_type = BCM_PORT_HG_TYPE_HGOE_TRANSPORT;
            } else if (mode == BCM_PORT_ENCAP_HIGIG) {
                tab_higig_type = BCM_PORT_HG_TYPE_HIGIGPLUS;
            } else {
                tab_higig_type = BCM_PORT_HG_TYPE_HIGIG2_NORMAL;
            }
        } else {
            tab_higig_type = 0;
        }

        if(SOC_REG_IS_VALID(unit, PGW_CELL_ASM_EMBEDDED_HG_CONTROL0r)) {
            uint64 field64;
            uint32 tmp32;
            tmp32 = (BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET == mode);
            COMPILER_64_SET(field64, 0, tmp32);
            if (SOC_FAILURE(soc_reg_get(unit, PGW_CELL_ASM_EMBEDDED_HG_CONTROL0r,
                            port, 0, &val64))) {
                _bcm_esw_port_mirror_unlock(unit);
                return rv;
            }
            soc_reg64_field_set(unit, PGW_CELL_ASM_EMBEDDED_HG_CONTROL0r,
                                &val64, TRANSPORT_ENf,
                                field64);
            rv = soc_reg_set(unit, PGW_CELL_ASM_EMBEDDED_HG_CONTROL0r, port, 0,
                             val64);
        }
        if (BCM_SUCCESS(rv) &&
            SOC_MEM_FIELD_VALID(unit, EGR_PORTm, PORT_TYPEf)) {
            rv = soc_mem_field32_modify(unit, EGR_PORTm, port, PORT_TYPEf, tab_port_type);
        }
        if (BCM_SUCCESS(rv) && SOC_MEM_FIELD_VALID(unit, EGR_PORTm, HG_TYPEf)) {
            rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                                        HG_TYPEf, tab_higig_type);
        }
        if (BCM_SUCCESS(rv) &&
            SOC_MEM_FIELD_VALID(unit, EGR_ING_PORTm, PORT_TYPEf)) {
            rv = soc_mem_field32_modify(unit, EGR_ING_PORTm, port, PORT_TYPEf, tab_port_type);

        }
        if (BCM_SUCCESS(rv) &&
            SOC_MEM_FIELD_VALID(unit, EGR_ING_PORTm, HG_TYPEf)) {
            rv = soc_mem_field32_modify(unit, EGR_ING_PORTm, port,
                                        HG_TYPEf, tab_higig_type);
        }

        if (BCM_SUCCESS(rv) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, PORT_TYPEf)) {
            rv = _bcm_esw_port_tab_set(unit, port,
                                       _BCM_CPU_TABS_NONE, PORT_TYPEf, tab_port_type);

        }
        if (BCM_SUCCESS(rv) && SOC_MEM_FIELD_VALID(unit, PORT_TABm, HG_TYPEf)) {

            rv = _bcm_esw_port_tab_set(unit, port,
                                       _BCM_CPU_TABS_NONE, HG_TYPEf, tab_higig_type); 
        }
#ifdef BCM_TRX_SUPPORT
        /* Reset the vlan default action, if not done already */
        if (xport_swap == FALSE) {
            if (soc_feature(unit, soc_feature_vlan_action)) {
                bcm_vlan_action_set_t action;

                if (BCM_FAILURE(_bcm_trx_vlan_port_egress_default_action_get(
                                    unit, port, &action))) {
                    _bcm_esw_port_mirror_unlock(unit);
                    return rv;
                }
                /* Backward compatible defaults */
                if ((mode == BCM_PORT_ENCAP_HIGIG) ||
                    (mode == BCM_PORT_ENCAP_HIGIG2) ||
                    (mode == BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET)) {
                    action.ot_outer = bcmVlanActionDelete;
                    action.dt_outer = bcmVlanActionDelete;
                } else {
                    action.ot_outer = bcmVlanActionNone;
                    action.dt_outer = bcmVlanActionNone;
                }
                if (BCM_FAILURE(_bcm_trx_vlan_port_egress_default_action_set(
                                    unit, port, &action))) {
                    _bcm_esw_port_mirror_unlock(unit);
                    return rv;
                }
            }
        }
#endif
    }

    /* Update cached version of HiGig2 encapsulation */
    if (BCM_SUCCESS(rv)) {
        if (mode == BCM_PORT_ENCAP_HIGIG2 ||
            mode == BCM_PORT_ENCAP_HIGIG2_LITE) {
            SOC_HG2_ENABLED_PORT_ADD(unit, port);
        } else {
            SOC_HG2_ENABLED_PORT_REMOVE(unit, port);
        }
    }

    size = SOC_INFO(unit).max_mtu;
    if ((mode == BCM_PORT_ENCAP_IEEE) ||
        (mode == BCM_PORT_ENCAP_HIGIG_OVER_ETHERNET) ||
        (mode == BCM_PORT_ENCAP_HIGIG2_L2) ||
        (mode == BCM_PORT_ENCAP_HIGIG2_IP_GRE)) {
        to_higig = 0;
        size -= 4;
    } else {
        to_higig = 1;
    }

    if (BCM_SUCCESS(rv)) {
        soc_xport_type_update(unit, port, to_higig);
        rv = bcmi_esw_portctrl_frame_max_set(unit, port, size);
    }

    _bcm_esw_port_mirror_unlock(unit);

    return rv;
}
#endif  /* PORTMOD_SUPPORT */

/*
 * Function:
 *      bcmi_portctrl_encap_set
 * Purpose:
 *      Set the port encapsulation mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      mode - One of BCM_PORT_ENCAP_xxx (see port.h)
 *      force - TRUE  : force encap operation(set xport_swap = TRUE)
 *                      regardless of IS_HG_PORT and mode value.
 *
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_encap_set(int unit, bcm_gport_t port, int mode, int force)
{
#ifdef PORTMOD_SUPPORT
    int pport;
    bcm_port_ability_t port_ability;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN(
        bcmi_esw_portctrl_ability_get(unit, port, &port_ability, NULL));

    if (!((port_ability.encap) & BCM_PA_ENCAP(mode))) {
        LOG_VERBOSE(BSL_LS_BCM_PORT,
                    (BSL_META_UP(unit, port,
                                "Encap mode %d not supported on port %d\n"),
                    mode, port));
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        bcmi_esw_portctrl_encap_set_execute(unit, port, mode, force));

    return BCM_E_NONE;
#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
*      bcmi_esw_portctrl_llfc_get
 * Purpose:
 *      
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      type - Command Type  
 *      value  - Value  
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      
 *      
 */

int
bcmi_esw_portctrl_llfc_get(int unit, bcm_gport_t port, bcm_port_control_t type, int *value)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_UNAVAIL;
    portmod_llfc_control_t control;

    PORT_LOCK(unit);
    switch(type) {
    case bcmPortControlLLFCReceive:
    case bcmPortControlSAFCReceive:
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            if (!SOC_PORT_VALID(unit, port)) {
                PORT_UNLOCK(unit);
                return BCM_E_PORT;
            }
            if (!IS_HG_PORT(unit, port)) {
                PORT_UNLOCK(unit);
                return BCM_E_UNAVAIL;
            }
            rv = portmod_port_llfc_control_get(unit, port, &control);
            *value = control.rx_enable;
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        break;
    case bcmPortControlLLFCTransmit:
    case bcmPortControlSAFCTransmit:
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            if (!SOC_PORT_VALID(unit, port)) {
                PORT_UNLOCK(unit);
                return BCM_E_PORT;
            }
            if (!IS_HG_PORT(unit, port)) {
                PORT_UNLOCK(unit);
                return BCM_E_UNAVAIL;
            }
            rv = portmod_port_llfc_control_get(unit, port, &control);
            *value = control.tx_enable;
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        break;
    default:
        break;
    }
    PORT_UNLOCK(unit);
    return(rv);
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_llfc_set
 * Purpose:
 *      
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      type - Command Type  
 *      value  - Value  
 *
 *
 */
int
bcmi_esw_portctrl_llfc_set(int unit, bcm_port_t port, bcm_port_control_t type, int value)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_UNAVAIL;
    soc_reg_t port_config;
    int llfc_enable = 0;
    portmod_llfc_control_t control;
#if defined(BCM_APACHE_SUPPORT)
    uint32 rval;
#endif

    switch(type) {
    case bcmPortControlLLFCReceive:
    case bcmPortControlSAFCReceive:
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {

            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
            if (!IS_HG_PORT(unit, port)) {
                return BCM_E_UNAVAIL;
            }
            PORT_LOCK(unit);
            rv = portmod_port_llfc_control_get(unit, port, &control);
            if(PORTMOD_SUCCESS(rv)) { 
                control.rx_enable = value;
                rv =  portmod_port_llfc_control_set(unit,port, &control);
                if(PORTMOD_SUCCESS(rv)) {
                    if( value == 0 ) {
                        rv = portmod_port_llfc_control_get(unit, port, &control);
                        if(PORTMOD_SUCCESS(rv)) {
                            llfc_enable = control.rx_enable ;
                        }
                    } else {
                        llfc_enable = TRUE;
                    }
                }
            }
            PORT_UNLOCK(unit);
            if (SOC_REG_FIELD_VALID(unit, XLPORT_CONFIGr, LLFC_ENf)) {
                port_config = XLPORT_CONFIGr;
            } else if (SOC_REG_FIELD_VALID(unit, XPORT_CONFIGr, LLFC_ENf)) {
                port_config = XPORT_CONFIGr;
            } else if (SOC_REG_FIELD_VALID(unit, PORT_CONFIGr, LLFC_ENf)) {
                port_config = PORT_CONFIGr;
            } else {
                port_config = INVALIDr;
            }

            if (port_config != INVALIDr){
                BCM_IF_ERROR_RETURN
                    (soc_reg_field32_modify(unit, port_config, port, LLFC_ENf,
                                            llfc_enable ? 1 : 0));
            }
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        break;
    case bcmPortControlLLFCTransmit:
    case bcmPortControlSAFCTransmit:
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (SOC_IS_XGS3_SWITCH(unit)) {
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
            if (!IS_HG_PORT(unit, port)) {
                return BCM_E_UNAVAIL;
            }
            PORT_LOCK(unit);
            rv = portmod_port_llfc_control_get(unit, port, &control);
            if(PORTMOD_SUCCESS(rv)) {
                control.tx_enable = value;
                rv =  portmod_port_llfc_control_set(unit,port, &control);
                if(PORTMOD_SUCCESS(rv)) {
                    if( value == 0 ) {
                        rv = portmod_port_llfc_control_get(unit, port, &control);
                        if(PORTMOD_SUCCESS(rv)) {
                            llfc_enable = control.tx_enable ;
                        }
                    } else {
                        llfc_enable = TRUE;
                    }
                }
            }
            PORT_UNLOCK(unit);
            if (SOC_REG_FIELD_VALID(unit, XLPORT_CONFIGr, LLFC_ENf)) {
                port_config = XLPORT_CONFIGr;
            } else if (SOC_REG_FIELD_VALID(unit, XPORT_CONFIGr, LLFC_ENf)) {
                port_config = XPORT_CONFIGr;
            } else if (SOC_REG_FIELD_VALID(unit, PORT_CONFIGr, LLFC_ENf)) {
                port_config = PORT_CONFIGr;
            } else {
                port_config = INVALIDr;
            }

            if (port_config != INVALIDr){
                BCM_IF_ERROR_RETURN
                    (soc_reg_field32_modify(unit, port_config, port, LLFC_ENf,
                                            llfc_enable ? 1 : 0));
            }
#if  defined(BCM_APACHE_SUPPORT)
            if ( SOC_IS_APACHE(unit)) {
                SOC_IF_ERROR_RETURN
                    (soc_reg32_get(unit, THDI_INPUT_PORT_XON_ENABLESr,
                                   port, 0, &rval));

                soc_reg_field_set(unit, THDI_INPUT_PORT_XON_ENABLESr, &rval,
                                  PORT_PRI_XON_ENABLEf,
                                  llfc_enable ? 0xffff : 0);
                soc_reg_field_set(unit, THDI_INPUT_PORT_XON_ENABLESr, &rval,
                                  PORT_PAUSE_ENABLEf, llfc_enable ? 0 : 1);

                SOC_IF_ERROR_RETURN
                    (soc_reg32_set(unit, THDI_INPUT_PORT_XON_ENABLESr,
                                   port, 0, rval));
            }
#endif
 
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
        break;
    default:
        break;
    }
    return rv;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_link_get
 * Purpose:
 *      Return current PHY up/down status
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      hw - If TRUE, assume hardware linkscan is active and use it
 *              to reduce PHY reads.
 *           If FALSE, do not use information from hardware linkscan.
 *      up - (OUT) TRUE for link up, FALSE for link down.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_link_get(int unit, bcm_gport_t port, int hw, int *up)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    int flags = 0;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);

    if (hw) {
        pbmp_t hw_linkstat;

        rv = soc_linkscan_hw_link_get(unit, &hw_linkstat);

        *up = PBMP_MEMBER(hw_linkstat, port);

        /*
         * We need to confirm link down because we may receive false link
         * change interrupts when hardware and software linkscan are mixed.
         * Processing a false link down event is known to cause packet
         * loss, which is obviously unacceptable.
         */
        if (!(*up)) {
            rv = portmod_port_link_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, up);
        }
    } else {
        if (SOC_IS_RCPU_ONLY(unit)) {
            PORTMOD_PORT_ENABLE_MAC_SET(flags);
            rv = portmod_port_enable_get(unit, pport, flags, up);
        } else {
            rv = portmod_port_link_get(unit, pport, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, up);
        }
    }

    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv) && 
        portmod_port_flags_test(unit, port, PHY_FLAGS_MEDIUM_CHANGE) == 1) {
        soc_port_medium_t medium;

        rv = bcmi_esw_portctrl_medium_get(unit, port, &medium);
        if (BCM_SUCCESS(rv)) {
            soc_phy_medium_status_notify(unit, port, medium);
        }
    }

    LOG_VERBOSE(BSL_LS_BCM_PORT,
                (BSL_META_UP(unit, port,
                    "Get port link status: u=%d p=%d hw=%d up=%d rv=%d\n"),
                 unit, port, hw, *up, rv));

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_lag_failover_disable
 * Purpose:
 *      Disable lag failover
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 */
int
bcmi_esw_portctrl_lag_failover_disable(int unit, bcm_gport_t port)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_lag_failover_disable(unit, pport);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_lag_failover_loopback_get
 * Purpose:
 *      Get lag failover loopback mode for specified port.
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      value  - Value
 */
int
bcmi_esw_portctrl_lag_failover_loopback_get(int unit,
                                            bcm_gport_t port, int* value)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_lag_failover_loopback_get(unit, pport, value);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_lag_failover_status_toggle
 * Purpose:
 *      Toggle lag failover status
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 */
int
bcmi_esw_portctrl_lag_failover_status_toggle(int unit, bcm_gport_t port)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_lag_failover_status_toggle(unit, pport);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_lag_remove_failover_lpbk_set
 * Purpose:
 *      Set lag remove failover loopback
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 */
int
bcmi_esw_portctrl_lag_remove_failover_lpbk_set(int unit,
                                               bcm_gport_t port, int value)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_lag_remove_failover_lpbk_set(unit, pport, value);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_trunk_hwfailover_set
 * Purpose:
 *      Set trunk hwfailover
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port.
 *      hw_count - hw count
 */
int
bcmi_esw_portctrl_trunk_hwfailover_set(int unit, bcm_gport_t port, int hw_count)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t  pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_trunk_hwfailover_config_set(unit, pport, hw_count);
    PORT_UNLOCK(unit);
    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

#ifdef BCM_TRIDENT2_SUPPORT
/*
 * Function:
 *      bcmi_esw_td2_portctrl_lanes_set
 * Purpose:
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port_base - Port Base.
 *      lanes  - lanes
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *
 *
 */
int
bcmi_esw_td2_portctrl_lanes_set(int unit, bcm_port_t port_base, int lanes)
{
#ifdef PORTMOD_SUPPORT
    soc_info_t *si = &SOC_INFO(unit);
    soc_td2_port_lanes_t lanes_ctrl;
    int port, i;
    int enable, okay;

    sal_memset(&lanes_ctrl, 0, sizeof(lanes_ctrl));
    lanes_ctrl.port_base = port_base;
    lanes_ctrl.lanes = lanes;
    SOC_IF_ERROR_RETURN
        (soc_trident2_port_lanes_validate(unit, &lanes_ctrl));

    if(lanes_ctrl.lanes == lanes_ctrl.cur_lanes) {
        return BCM_E_NONE;
    }

    /* All existing ports are required to be disasbled */
    SOC_IF_ERROR_RETURN(bcm_esw_port_enable_get(unit, port_base, &enable));
    if (enable) {
        return BCM_E_BUSY;
    }
    if(lanes_ctrl.lanes > lanes_ctrl.cur_lanes) { /* port(s) to be removed */
        for (i = 0; i < lanes_ctrl.phy_ports_len; i++) {
            port = si->port_p2l_mapping[lanes_ctrl.phy_ports[i]];
            SOC_IF_ERROR_RETURN(bcm_esw_port_enable_get(unit, port, &enable));
            if (enable) {
                return BCM_E_BUSY;
            }
        }
    }
    SOC_IF_ERROR_RETURN
        (soc_trident2_port_lanes_set(unit, &lanes_ctrl));

    /* Probe PHY on all port(s) after conversion */
    BCM_IF_ERROR_RETURN(_bcm_port_probe(unit, port_base, &okay));
    BCM_IF_ERROR_RETURN(_bcm_port_mode_setup(unit, port_base, FALSE));
    if(lanes_ctrl.lanes < lanes_ctrl.cur_lanes) { /* port(s) to be added */
        for (i = 0; i < lanes_ctrl.phy_ports_len; i++) {
            port = si->port_p2l_mapping[lanes_ctrl.phy_ports[i]];
            BCM_IF_ERROR_RETURN(_bcm_port_probe(unit, port, &okay));
            BCM_IF_ERROR_RETURN(_bcm_port_mode_setup(unit, port, FALSE));
        }
    }

    return BCM_E_NONE;
#else /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */

}
#endif /* BCM_TRIDENT2_SUPPORT */


/*
 * Function:
 *      bcmi_esw_portctrl_hwfailover_enable_set
 * Purpose:
 *      Set front panel trunk hardware failover config.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      port - (IN) Port ID of the fail port.
 *      enable - (IN) Set the HW failover config.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_hwfailover_enable_set(int unit, bcm_gport_t port, int enable)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_trunk_hwfailover_config_set(unit, pport, enable);
    PORT_UNLOCK(unit);

    if (PORTMOD_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                    (BSL_META_UP(unit, port,
                        "Failed to set HW failover: u=%d p=%d rv=%d\n"),
                    unit, port, rv));
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_hwfailover_enable_get
 * Purpose:
 *      Get front-panel trunk hardware failover config.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      port - (IN) Port ID of the fail port.
 *      enable - (OUT) Get the HW failover config.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_hwfailover_enable_get(int unit, bcm_gport_t port, int *enable)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_trunk_hwfailover_config_get(unit, pport, enable);
    PORT_UNLOCK(unit);

    if (PORTMOD_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                    (BSL_META_UP(unit, port,
                        "Failed to get HW failover config: u=%d p=%d rv=%d\n"),
                    unit, port, rv));
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_hwfailover_status_get
 * Purpose:
 *      Get front-panel trunk hardware failover status.
 * Parameters:
 *      unit - (IN) SOC unit number. 
 *      port - (IN) Port ID of the fail port.
 *      status - (OUT) failover status.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_hwfailover_status_get(int unit, bcm_gport_t port, int *status)
{
#ifdef PORTMOD_SUPPORT
    int rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_trunk_hwfailover_status_get(unit, pport, status);
    PORT_UNLOCK(unit);

    if (PORTMOD_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                    (BSL_META_UP(unit, port,
                        "Failed to get HW failover status: u=%d p=%d rv=%d\n"),
                    unit, port, rv));
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_phy_control_get
 * Description:
 *     Set PHY specific properties
 * Parameters:
 *     unit        device number
 *     port        port number
 *     type        configuration type
 *     value       value for the configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_control_get(int unit, bcm_gport_t port,
                                  bcm_port_phy_control_t type, uint32 *value)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;
    int phy_lane = -1;
    int phyn = -1, sys_side = 0;
    bcm_port_t local_port = -1;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_gport_phyn_validate(unit, port,
                                           &local_port, &phyn,
                                           &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);

    if (local_port == -1) {
        /* Configure outermost PHY (common case) */
        rv = soc_portctrl_phy_control_get(unit, pport,
                                          -1, -1, 0,
                                          (soc_phy_control_t) type, value);
    } else {
        /* Configure PHY specified by GPORT */
        rv = soc_portctrl_phy_control_get(unit, pport,
                                          phyn, phy_lane, sys_side,
                                          (soc_phy_control_t) type, value);
    }

    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_phy_control_set
 * Description:
 *     Set PHY specific properties
 * Parameters:
 *     unit        device number
 *     port        port number
 *     type        configuration type
 *     value       new value for the configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcmi_esw_portctrl_phy_control_set(int unit, bcm_gport_t port,
                                 bcm_port_phy_control_t type, uint32 value)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;
    int phy_lane = -1;
    int phyn = -1, sys_side = 0;
    bcm_port_t local_port = -1;
    _bcm_port_info_t *port_info;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_gport_phyn_validate(unit, port,
                                           &local_port, &phyn,
                                           &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);

    if (local_port == -1) {
        /* Configure outermost PHY (common case) */
        rv = soc_portctrl_phy_control_set(unit, pport,
                                          -1, -1, 0,
                                          (soc_phy_control_t) type, value);
    } else {
        /* Configure PHY specified by GPORT */
        rv = soc_portctrl_phy_control_set(unit, pport,
                                          phyn, phy_lane, sys_side,
                                          (soc_phy_control_t) type, value);
    }

    /* Provide Warmboot support for SW RX LOS status */
    if (SOC_SUCCESS(rv) && 
        (type == BCM_PORT_PHY_CONTROL_SOFTWARE_RX_LOS)) {
        _bcm_port_info_access(unit, port, &port_info);
        port_info->rx_los = value;
    }

    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


#ifdef PORTMOD_SUPPORT
/*
 * Function:
 *      _bcm_esw_portctrl_control_pfc_receive_set
 * Purpose:
 *      Set the bcmPortControlPFCReceive port control.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) BCM port number (NONE gport format).
 *      pport     - (IN) Portmod port number.
 *      value     - (IN) value to be set.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Assumes caller will:
 *      - Check for PORTCTRL_INIT()
 *      - Provide LOCKs
 */
STATIC int
_bcm_esw_portctrl_control_pfc_receive_set(int unit, bcm_port_t port,
                                          portctrl_pport_t pport,
                                          int value)
{
    int pfc_enable;
    portmod_pfc_control_t pfc_control;

    if (!soc_feature(unit, soc_feature_priority_flow_control)) {
        return BCM_E_UNAVAIL;
    }

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_get(unit, pport, &pfc_control));

    pfc_control.rx_enable = value;            
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_set(unit, pport, &pfc_control));

    if (value == 0) {
        pfc_enable = pfc_control.tx_enable;
    } else {
        pfc_enable = TRUE;
    }

    if (value == 0) {
        /* Disabling RX, flush MMU XOFF state */
        pfc_control.force_xon = 1;
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_pfc_control_set(unit, pport, &pfc_control));
        pfc_control.force_xon = 0;
        PORTMOD_IF_ERROR_RETURN
            (portmod_port_pfc_control_set(unit, pport, &pfc_control));
    }

    /* Stats enable */
    pfc_control.stats_en = pfc_enable ? 1 : 0;
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_set(unit, pport, &pfc_control));
    
    if (SOC_REG_IS_VALID(unit, XPORT_TO_MMU_BKPr) && (pfc_enable == 0)) {
        BCM_IF_ERROR_RETURN(WRITE_XPORT_TO_MMU_BKPr(unit, port, 0));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_control_pfc_transmit_set
 * Purpose:
 *      Set the bcmPortControlPFCTransmit port control.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) BCM port number (NONE gport format).
 *      pport     - (IN) Portmod port number.
 *      value     - (IN) value to be set.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Assumes caller will:
 *      - Check for PORTCTRL_INIT()
 *      - Provide LOCKs
 */
STATIC int
_bcm_esw_portctrl_control_pfc_transmit_set(int unit, bcm_port_t port,
                                           portctrl_pport_t pport,
                                           int value)
{
    int pfc_enable;
    portmod_pfc_control_t pfc_control;
    uint32 rval;

    if (!soc_feature(unit, soc_feature_priority_flow_control)) {
        return BCM_E_UNAVAIL;
    }

    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_get(unit, pport, &pfc_control));

    pfc_control.tx_enable = value;            
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_set(unit, pport, &pfc_control));

    if (value == 0) {
        pfc_enable = pfc_control.rx_enable;
    } else {
        pfc_enable = TRUE;
    }

    /* Port config settings */
    if (SOC_REG_IS_VALID(unit, THDI_INPUT_PORT_XON_ENABLESr)) {
        SOC_IF_ERROR_RETURN
            (soc_reg32_get(unit, THDI_INPUT_PORT_XON_ENABLESr,
                           port, 0, &rval));
        soc_reg_field_set(unit, THDI_INPUT_PORT_XON_ENABLESr, &rval,
                          PORT_PRI_XON_ENABLEf,
                          pfc_enable ? 0xffff : 0);
        soc_reg_field_set(unit, THDI_INPUT_PORT_XON_ENABLESr, &rval,
                          PORT_PAUSE_ENABLEf, pfc_enable ? 0 : 1);
        SOC_IF_ERROR_RETURN
            (soc_reg32_set(unit, THDI_INPUT_PORT_XON_ENABLESr,
                           port, 0, rval));
    }

    /* Stats enable */
    pfc_control.stats_en = pfc_enable ? 1 : 0;
    PORTMOD_IF_ERROR_RETURN
        (portmod_port_pfc_control_set(unit, pport, &pfc_control));

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_portctrl_control_eee_enable_set
 * Purpose:
 *      Set the bcmPortControlEEEEnable port control.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) BCM port number (NONE gport format).
 *      pport     - (IN) Portmod port number.
 *      value     - (IN) value to be set.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Assumes caller will:
 *      - Check for PORTCTRL_INIT()
 *      - Provide LOCKs
 */
STATIC int
_bcm_esw_portctrl_control_eee_enable_set(int unit, bcm_port_t port,
                                         portctrl_pport_t pport,
                                         int value)
{
#if 0
    
    portmod_eee_t eee;
    uint32 phy_val;

    if (!soc_feature (unit, soc_feature_eee)) {
        return BCM_E_UNAVAIL;
    }

    PORTMOD_IF_ERROR_RETURN(portmod_eee_t_init(unit, &eee));

    rv = portmod_port_eee_get(unit, pport, &eee);
                            
    
    
    if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                         SOC_MAC_CONTROL_EEE_ENABLE,
                         &mac_val) != SOC_E_UNAVAIL) &&
        (soc_phyctrl_control_get(unit, port,
                                 BCM_PORT_PHY_CONTROL_EEE,
                                 &phy_val) != SOC_E_UNAVAIL)) {

        /* If MAC/Switch is EEE aware (Native EEE mode is supported)
         * and PHY also supports Native EEE mode
         */

        /* a. Disable AutoGrEEEn mode by PHY if applicable */
        rv = (soc_phyctrl_control_get(unit, port,
                                      BCM_PORT_PHY_CONTROL_EEE_AUTO,
                                      &phy_val));

        if ((rv != SOC_E_UNAVAIL) && (phy_val != 0)) {
            rv = soc_phyctrl_control_set(unit, port,
                                         BCM_PORT_PHY_CONTROL_EEE_AUTO,
                                         0);
        }

        /* b. Enable/Disable Native EEE in PHY */
        rv = soc_phyctrl_control_set (unit, port,
                                      BCM_PORT_PHY_CONTROL_EEE, value ? 1 : 0);

        if (SOC_SUCCESS(rv)) {
            /* EEE standard compliance Work Around:
             * Store the software copy of eee value in eee_cfg flag
             */
            eee_cfg[unit][port] = value;
            /* If (value==1), EEE will be enabled in MAC after 1 sec.
             * during linkscan update*/
            if (value == 0) {
                /* Disable EEE in MAC immediately*/
                rv = MAC_CONTROL_SET(PORT(unit, port).p_mac,
                                     unit, port,
                                     SOC_MAC_CONTROL_EEE_ENABLE, 0);
            }

            /* Notify Int-PHY to bypass LPI for native EEE mode.
             *
             * Note :
             *  1. Not all internal SerDes support the setting to
             *     enable/disable bypass LPI signal.
             *  2. Int-PHY to bypass LPI will sync with Ext-PHY's EEE
             *     enabling status for Native EEE mode.
             */
            (void)soc_phyctrl_notify(unit, port,
                                     phyEventLpiBypass, value? 1: 0);
        }

    } else {
        /* If native EEE mode is not supported,
         * set PHY in AutoGrEEEn mode.
         */

        /* a. Disable Native EEE mode in PHY if applicable */
        rv = (soc_phyctrl_control_get(unit, port,
                                      BCM_PORT_PHY_CONTROL_EEE,
                                      &phy_val));

        if ((rv != SOC_E_UNAVAIL) && (phy_val != 0)) {
            rv = soc_phyctrl_control_set (unit, port,
                                          BCM_PORT_PHY_CONTROL_EEE,
                                          0);
        }

        /* b. Enable/Disable AutoGrEEEn in PHY.
         * If PHY does not support AutoGrEEEn mode,
         * rv will be assigned SOC_E_UNAVAIL.
         */
        rv = soc_phyctrl_control_set (unit, port,
                                      BCM_PORT_PHY_CONTROL_EEE_AUTO,
                                      value ? 1 : 0);
        if (SOC_SUCCESS(rv)) {
            eee_cfg[unit][port] = value;
        }
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_portctrl_eee_statistics_clear
 * Purpose:
 *      Set the bcmPortControlEEEStatisticsClear port control.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) BCM port number (NONE gport format).
 *      pport     - (IN) Portmod port number.
 *      value     - (IN) value to be set.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Assumes caller will:
 *      - Check for PORTCTRL_INIT()
 *      - Provide LOCKs
 */
STATIC int
_bcm_esw_portctrl_eee_statistics_clear(int unit, bcm_port_t port,
                                         portctrl_pport_t pport,
                                         int value)
{

#if 0
    uint32 val = 0;
    uint64 val64;
    uint32 phy_val;
    int idx;
    soc_reg_t regs[] = {
        RX_EEE_LPI_DURATION_COUNTERr,
        RX_EEE_LPI_EVENT_COUNTERr,
        TX_EEE_LPI_DURATION_COUNTERr,
        TX_EEE_LPI_EVENT_COUNTERr
    };

    if (!soc_feature(unit, soc_feature_eee)) {
        return BCM_E_UNAVAIL;
    }

    COMPILER_64_ZERO(val64);
    if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                         SOC_MAC_CONTROL_EEE_ENABLE, &mac_val)
         != SOC_E_UNAVAIL) &&
        (soc_phyctrl_control_get(unit, port,
                                 BCM_PORT_PHY_CONTROL_EEE, &phy_val)
         != SOC_E_UNAVAIL)) {

        only for TD2...
            for (idx = 0; idx < sizeof(regs) / sizeof(regs[0]); idx++) {
                SOC_IF_ERROR_RETURN
                    (soc_counter_set(unit, port, regs[idx], 0, val64));
            }
        return ??

        /* MAC/Switch is EEE aware (Native EEE mode is supported) */
        if (soc_feature(unit, soc_feature_unified_port)) {
            if (SOC_REG_IS_VALID(unit, PORT_XGXS_COUNTER_MODEr)) {
                _BCM_PORT_IF_ERROR_RETURN_WITH_UNLOCK
                    (READ_PORT_XGXS_COUNTER_MODEr(unit, port, &val));
                soc_reg_field_set(unit, PORT_XGXS_COUNTER_MODEr, &val,
                                  CNT_MODEf, 0);
                _BCM_PORT_IF_ERROR_RETURN_WITH_UNLOCK
                    (WRITE_PORT_XGXS_COUNTER_MODEr(unit, port, val));
            }
        } 

        /* Read counters to clear them*/
        SOC_IF_ERROR_RETURN
            (READ_RX_EEE_LPI_DURATION_COUNTERr(unit, port, &val64));
        SOC_IF_ERROR_RETURN
            (READ_RX_EEE_LPI_EVENT_COUNTERr(unit, port, &val64));
        SOC_IF_ERROR_RETURN
            (READ_TX_EEE_LPI_DURATION_COUNTERr(unit, port, &val64));
        SOC_IF_ERROR_RETURN
            (READ_TX_EEE_LPI_EVENT_COUNTERr(unit, port, &val64));

        /* Set counter rollover bit to 1*/
        if (soc_feature(unit, soc_feature_unified_port)) {
            if (SOC_REG_IS_VALID(unit,
                                 PORT_XGXS_COUNTER_MODEr)) {
                SOC_IF_ERROR_RETURN
                    (READ_PORT_XGXS_COUNTER_MODEr(unit, port, &val));
                soc_reg_field_set(unit, PORT_XGXS_COUNTER_MODEr, &val,
                                  CNT_MODEf, 1);
                SOC_IF_ERROR_RETURN
                    (WRITE_PORT_XGXS_COUNTER_MODEr(unit, port, val));
            }
        }
    } else {
        /* native EEE mode is not supported, */
        rv = soc_phyctrl_control_set
            (unit, port,
             BCM_PORT_PHY_CONTROL_EEE_STATISTICS_CLEAR, value);
    }
#endif

    return BCM_E_NONE;
}
#endif /* PORTMOD_SUPPORT */


/*
 * Function:
 *      bcmi_esw_portctrl_control_validate
 * Purpose:
 *      Check that given port control feature in valid in Port Control.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      type    - (IN) Port control feature bcmPortControlxxx.
 *      valid   - (OUT) Indicates if control feature is valid.
 * Returns:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_control_validate(int unit, bcm_port_control_t type,
                                   int *valid)
{
#ifdef PORTMOD_SUPPORT

    PORTCTRL_INIT_CHECK(unit);

    *valid = 0;
    switch (type) {
    case bcmPortControlEEEEnable:
    case bcmPortControlEEEReceiveDuration:
    case bcmPortControlEEEReceiveEventCount:
    case bcmPortControlEEEReceiveQuietTime:
    case bcmPortControlEEEReceiveSleepTime:
    case bcmPortControlEEEReceiveWakeTime:
    case bcmPortControlEEEStatisticsClear:
    case bcmPortControlEEETransmitDuration:
    case bcmPortControlEEETransmitEventCount:
    case bcmPortControlEEETransmitIdleTime:
    case bcmPortControlEEETransmitQuietTime:
    case bcmPortControlEEETransmitRefreshTime:
    case bcmPortControlEEETransmitSleepTime:
    case bcmPortControlEEETransmitWakeTime:
    case bcmPortControlFrameSpacingStretch:
    case bcmPortControlLinkdownTransmit:
    case bcmPortControlLinkFaultLocal:
    case bcmPortControlLinkFaultLocalEnable:
    case bcmPortControlLinkFaultRemote:
    case bcmPortControlLinkFaultRemoteEnable:
    case bcmPortControlPassControlFrames:
    case bcmPortControlPFCClasses:
    case bcmPortControlPFCDestMacNonOui:
    case bcmPortControlPFCDestMacOui:
    case bcmPortControlPFCEthertype:
    case bcmPortControlPFCOpcode:
    case bcmPortControlPFCPassFrames:
    case bcmPortControlPFCReceive:
    case bcmPortControlPFCRefreshTime:
    case bcmPortControlPFCTransmit:
    case bcmPortControlPFCXOffTime:
    case bcmPortControlPrbsPolynomial:
    case bcmPortControlPrbsRxEnable:
    case bcmPortControlPrbsRxStatus:
    case bcmPortControlPrbsTxEnable:
    case bcmPortControlPrbsTxInvertData:
    case bcmPortControlRxEnable:
    case bcmPortControlSerdesDriverTune:
    case bcmPortControlSerdesDriverEqualizationTuneStatusFarEnd:
    case bcmPortControlSerdesTuneMarginMax:
    case bcmPortControlSerdesTuneMarginMode:
    case bcmPortControlSerdesTuneMarginValue:
    case bcmPortControlTimestampTransmit:
        *valid = 1;
        break;
    default:
        *valid = 0;
        break;
    }

    return BCM_E_NONE;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_control_set
 * Purpose:
 *      Set specified port control feature.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) Port number.
 *      type      - (IN) Port control feature to set.
 *      value     - (IN) Value of port control feature.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Any new control needs to be included in
 *      bcmi_esw_portctrl_control_validate().
 */
int
bcmi_esw_portctrl_control_set(int unit, bcm_gport_t port,
                              bcm_port_control_t type, int value)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_UNAVAIL;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    switch (type) {
    case bcmPortControlPassControlFrames:
        if (IS_E_PORT(unit, port)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_NONE,
                                       PASS_CONTROL_FRAMESf,
                                       (value) ? 1 : 0);
        }

        if (IS_XE_PORT(unit, port) || IS_CE_PORT(unit, port) ||
            IS_C_PORT(unit, port) ||
            (IS_GE_PORT(unit, port)
             && soc_feature(unit, soc_feature_unified_port))) {
            /* Enable Control Frames in BigMAC */
            if(soc_feature(unit, soc_feature_pgw_mac_control_frame)) {
                uint32 rval32 = 0;

                PORT_LOCK(unit);
                rv = READ_PGW_MAC_RSV_MASKr(unit, port, &rval32);
                if (BCM_SUCCESS(rv)) {
                    /* PGW_MAC_RSV_MASK: Bit 11 Control Frame recieved
                     * Enable  Control Frame : Set 0. Packet go through
                     * Disable Control Frame : Set 1. Packet is purged.
                     */
                    if(value) {
                        rval32 &= ~(1 << 11);
                    } else {
                        rval32 |= (1 << 11);
                    }
                    rv = WRITE_PGW_MAC_RSV_MASKr(unit, port, rval32);
                }
                PORT_UNLOCK(unit);
            } else {
                portmod_rx_control_t rx_control;

                PORTMOD_IF_ERROR_RETURN
                    (portmod_rx_control_t_init(unit, &rx_control));

                rx_control.flags = PORTMOD_MAC_PASS_CONTROL_FRAME;
                rx_control.pass_control_frames = value;
                PORT_LOCK(unit);
                rv = portmod_port_rx_control_set(unit, pport, &rx_control);
                PORT_UNLOCK(unit);
            }
        }
        break;

    case bcmPortControlFrameSpacingStretch:
        PORT_LOCK(unit);
        rv = portmod_port_frame_spacing_stretch_set(unit, pport, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlPFCEthertype:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_config.type = value;
                rv = portmod_port_pfc_config_set(unit, pport, &pfc_config);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPFCOpcode:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_config.opcode = value;
                rv = portmod_port_pfc_config_set(unit, pport, &pfc_config);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPFCReceive:
        PORT_LOCK(unit);
        rv = _bcm_esw_portctrl_control_pfc_receive_set(unit, port,
                                                       pport, value);
        PORT_UNLOCK(unit); 
        break;

    case bcmPortControlPFCTransmit:
        PORT_LOCK(unit);
        rv = _bcm_esw_portctrl_control_pfc_transmit_set(unit, port,
                                                        pport, value);
        PORT_UNLOCK(unit); 
        break;

    case bcmPortControlPFCClasses:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            if (value < 0 || value > 0xffffff) {
                return BCM_E_PARAM;
            }

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_config.classes = value;
                rv = portmod_port_pfc_config_set(unit, pport, &pfc_config);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPFCPassFrames:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if(soc_feature(unit, soc_feature_pgw_mac_pfc_frame)) {
                uint32 rval32 = 0;
                PORT_LOCK(unit);
                rv = READ_PGW_MAC_RSV_MASKr(unit, port, &rval32);
                if (BCM_SUCCESS(rv)) {
                    /* PGW_MAC_RSV_MASK: Bit 18 PFC frame detected
                     * Enable  PFC Frame : Set 0. Go through
                     * Disable PFC Frame : Set 1. Purged.
                     */
                    if(value) {
                        rval32 &= ~(1 << 18);
                    } else {
                        rval32 |= (1 << 18);
                    }
                    rv = WRITE_PGW_MAC_RSV_MASKr(unit, port, rval32);
                }
                PORT_UNLOCK(unit);
            } else {
                portmod_pfc_config_t pfc_config;

                PORT_LOCK(unit);
                rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
                if (PORTMOD_SUCCESS(rv)) {
                    pfc_config.rxpass = value;
                    rv = portmod_port_pfc_config_set(unit, pport, &pfc_config);
                }
                PORT_UNLOCK(unit);
            }
        }
        break;

    case bcmPortControlPFCDestMacOui:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            if (value < 0 || value > 0xffffff) {
                return BCM_E_PARAM;
            }
            
            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_config.da_oui = value;
                rv = portmod_port_pfc_config_set(unit, pport, &pfc_config);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPFCDestMacNonOui:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            if (value < 0 || value > 0xffffff) {
                return BCM_E_PARAM;
            }

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_config.da_nonoui = value;
                rv = portmod_port_pfc_config_set(unit, pport, &pfc_config);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPFCRefreshTime:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_control_t pfc_control;

            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }

            PORT_LOCK(unit);
            rv = portmod_port_pfc_control_get(unit, pport, &pfc_control);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_control.refresh_timer = value;
                rv = portmod_port_pfc_control_set(unit, pport, &pfc_control);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPFCXOffTime:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_control_t pfc_control;

            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }

            PORT_LOCK(unit);
            rv = portmod_port_pfc_control_get(unit, pport, &pfc_control);
            if (PORTMOD_SUCCESS(rv)) {
                pfc_control.xoff_timer = value;
                rv = portmod_port_pfc_control_set(unit, pport, &pfc_control);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlPrbsPolynomial:
        PORT_LOCK(unit);
        rv = bcmi_esw_portctrl_phy_control_set(unit, port, 
             BCM_PORT_PHY_CONTROL_PRBS_POLYNOMIAL, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlPrbsTxEnable:
        PORT_LOCK(unit);
        rv = bcmi_esw_portctrl_phy_control_set(unit, port, 
             BCM_PORT_PHY_CONTROL_PRBS_TX_ENABLE, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlPrbsRxEnable:
        PORT_LOCK(unit);
        rv = bcmi_esw_portctrl_phy_control_set(unit, port, 
             BCM_ESW_PORTCTRL_PHYCTRL_PRBS_RX_ENABLE, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlPrbsTxInvertData:
        PORT_LOCK(unit);
        rv = bcmi_esw_portctrl_phy_control_set(unit, port, 
             BCM_PORT_PHY_CONTROL_PRBS_TX_INVERT_DATA, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlLinkFaultLocalEnable:
        {
            portmod_local_fault_control_t control;

            PORT_LOCK(unit);
            rv = portmod_port_local_fault_control_get(unit, pport, &control);
            if (PORTMOD_SUCCESS(rv)) {
                control.enable = value;
                rv = portmod_port_local_fault_control_set(unit,
                                                          pport, &control);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlLinkFaultRemoteEnable:
        {
            portmod_remote_fault_control_t control;

            PORT_LOCK(unit);
            rv = portmod_port_remote_fault_control_get(unit, pport, &control);
            if (PORTMOD_SUCCESS(rv)) {
                control.enable = value;
                rv = portmod_port_remote_fault_control_set(unit,
                                                           pport, &control);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlSerdesDriverTune:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           SOC_PHY_CONTROL_SERDES_DRIVER_TUNE, value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlLinkdownTransmit:
        PORT_LOCK(unit);
        rv = bcmi_esw_portctrl_phy_control_set(unit, port,
                 BCM_PORT_PHY_CONTROL_LINKDOWN_TRANSMIT, value);
        PORT_UNLOCK(unit);
        if (rv == BCM_E_NONE) {
            portmod_local_fault_control_t control;

            rv = _bcm_esw_link_down_tx_set(unit, port, value);

            PORT_LOCK(unit);
            rv = portmod_port_local_fault_control_get(unit, pport, &control);
            if (PORTMOD_SUCCESS(rv)) {
                control.enable = value;
                rv = portmod_port_local_fault_control_set(unit,
                                                          pport, &control);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlSerdesTuneMarginMode:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE, value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlSerdesTuneMarginValue:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE, value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlEEEEnable:
        PORT_LOCK(unit);
        
        rv = _bcm_esw_portctrl_control_eee_enable_set(unit, port,
                                                      pport, value);
        PORT_UNLOCK(unit);

        break;

    case bcmPortControlEEEStatisticsClear:
        PORT_LOCK(unit);
        
        rv = _bcm_esw_portctrl_eee_statistics_clear(unit, port,
                                                    pport, value);
        PORT_UNLOCK (unit);
        break;


 /**********************      EEE Mode Overview    ************************
 *                      |DET|                              |  WT |
 *   Signalling   |idles|   |------------------------------|     | idles   |
 *   from Tx MAC  | or  |   |   Low Power Idle (LPI)       |idles|  or     |
 *   to local PHY |data |   |------------------------------|     | data    |
 *                          *                              *
 *                          *                              *
 *                          *  -------LPI state------------*
 *   Local PHY    |         |  |      |  |       |  |      |   |           |
 *   signaling    |   Active|Ts|  Tq  |Tr|  Tq   |Tr|  Tq  |Tw |Active     |
 *   on MDI       |         |  |      |  |       |  |      |   |           |
 *                          *------------------------------*
 *                          *                              *
 *                          *                               *
 *   Signaling    |   idles |-------------------------------|id| PHY is    |
 *   from LP PHY  |     or  |  Low Power Idle (LPI)         |le| ready     |
 *   to Rx MAC    |   data  |-------------------------------|s | for data  |
 *
 *   where DET = Delay Entry Timer    WT = Tx MAC Wake Timer
 */
    case bcmPortControlEEETransmitIdleTime:
        /* DET = Time (in microsecs) for which condition to move to LPI state
         * is satisfied, at the end of which MAC TX transitions to LPI state */
        if (soc_feature (unit, soc_feature_eee)) {
            portmod_eee_t eee;

            PORT_LOCK(unit);
            rv = portmod_port_eee_get(unit, pport, &eee);
            if (PORTMOD_SUCCESS(rv)) {
                eee.tx_idle_time = value;
                rv = portmod_port_eee_set(unit, pport, &eee);
            }
            PORT_UNLOCK(unit);
        }
        break;

    case bcmPortControlEEETransmitRefreshTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_REFRESH_TIME, value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitSleepTime:
        PORT_LOCK (unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_SLEEP_TIME, value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitQuietTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_QUIET_TIME, value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitWakeTime:
        /* Time(in microsecs) to wait before transmitter can leave LPI State*/
        if (soc_feature (unit, soc_feature_eee)) {
            portmod_eee_t eee;

            PORT_LOCK(unit);
            rv = portmod_port_eee_get(unit, pport, &eee);
            if (PORTMOD_SUCCESS(rv)) {
                eee.tx_wake_time = value;
                rv = portmod_port_eee_set(unit, pport, &eee);
            }
            PORT_UNLOCK(unit);
        }
        break;

     case bcmPortControlEEEReceiveSleepTime:
        PORT_LOCK (unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_RECEIVE_SLEEP_TIME, value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEEReceiveQuietTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_set(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_RECEIVE_QUIET_TIME, value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlEEEReceiveWakeTime:
        PORT_LOCK(unit);

        
        /* rv = soc_phyctrl_control_set (unit, port,
           BCM_PORT_PHY_CONTROL_EEE_RECEIVE_WAKE_TIME, value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlRxEnable:
        rv = bcmi_esw_portctrl_mac_rx_control(unit, port, 0, &value);
        break;

    default:
        /* Control is not supported */
        break;
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}


/*
 * Function:
 *      bcmi_esw_portctrl_control_get
 * Purpose:
 *      Get the status of the specified port control feature.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) Port number.
 *      type      - (IN) Port control.
 *      value     - (OUT) Value of port control feature.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Any new control needs to be included in
 *      bcmi_esw_portctrl_control_validate().
 */
int
bcmi_esw_portctrl_control_get(int unit, bcm_gport_t port,
                              bcm_port_control_t type, int *value)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_UNAVAIL;
    portctrl_pport_t pport;

    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    switch (type) {
    case bcmPortControlPassControlFrames:
        if (IS_XE_PORT(unit, port) || IS_CE_PORT(unit, port) ||
            IS_C_PORT(unit, port) ||
            (IS_GE_PORT(unit, port)
             && soc_feature(unit, soc_feature_unified_port))) {
            if(soc_feature(unit, soc_feature_pgw_mac_control_frame)) {
                uint32 rval32 = 0;
                rv = READ_PGW_MAC_RSV_MASKr(unit, port, &rval32);
                if (BCM_SUCCESS(rv)) {
                    /* PGW_MAC_RSV_MASK: Bit 11 Control Frame recieved
                     * the bit value needs to be reversed
                     */
                    *value = ((rval32 & 0x800) >> 11) ? 0 : 1 ;
                }
            } else {
                portmod_rx_control_t rx_control;
                PORT_LOCK(unit);
                rv = portmod_port_rx_control_get(unit, pport, &rx_control);
                PORT_UNLOCK(unit);
                if (PORTMOD_SUCCESS(rv)) {
                    *value = rx_control.pass_control_frames;
                }
            }
        }
        break;

    case bcmPortControlFrameSpacingStretch:
        PORT_LOCK(unit);
        rv = portmod_port_frame_spacing_stretch_get(unit, pport, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlPFCEthertype:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_config.type;
            }
        }
        break;

    case bcmPortControlPFCOpcode:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_config.opcode;
            }
        }
        break;

    case bcmPortControlPFCReceive:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_control_t pfc_control;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_control_get(unit, pport, &pfc_control);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_control.rx_enable;
            }
        }
        break;

    case bcmPortControlPFCTransmit:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_control_t pfc_control;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_control_get(unit, pport, &pfc_control);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_control.tx_enable;
            }
        }
        break;

    case bcmPortControlPFCClasses:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_config.classes;
            }
        }
        break;

    case bcmPortControlPFCPassFrames:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if(soc_feature(unit, soc_feature_pgw_mac_pfc_frame)) {
                uint32 rval32 = 0;
                rv = READ_PGW_MAC_RSV_MASKr(unit, port, &rval32);
                if (BCM_SUCCESS(rv)) {
                    /* PGW_MAC_RSV_MASK: Bit 18 PFC frame detected
                     * the bit value needs to be reversed
                     */
                    *value = ((rval32 & 0x40000) >> 18) ? 0 : 1 ;
                }
            } else {
                portmod_pfc_config_t pfc_config;

                PORT_LOCK(unit);
                rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
                PORT_UNLOCK(unit);
                if (PORTMOD_SUCCESS(rv)) {
                    *value = pfc_config.rxpass;
                }
            }
        }
        break;

    case bcmPortControlPFCDestMacOui:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_config.da_oui;
            }
        }
        break;

    case bcmPortControlPFCDestMacNonOui:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_config_t pfc_config;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_config_get(unit, pport, &pfc_config);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_config.da_nonoui;
            }
        }
        break;

    case bcmPortControlPFCRefreshTime:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_control_t pfc_control;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_control_get(unit, pport, &pfc_control);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_control.refresh_timer;
            }
        }
        break;

    case bcmPortControlPFCXOffTime:
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            portmod_pfc_control_t pfc_control;

            PORT_LOCK(unit);
            rv = portmod_port_pfc_control_get(unit, pport, &pfc_control);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = pfc_control.xoff_timer;
            }
        }
        break;

    case bcmPortControlPrbsPolynomial:
        rv = bcmi_esw_portctrl_phy_control_get(unit, port,
             BCM_PORT_PHY_CONTROL_PRBS_POLYNOMIAL, (uint32 *)value);
        break;
    case bcmPortControlPrbsTxEnable:
        rv = bcmi_esw_portctrl_phy_control_get(unit, port, 
             BCM_PORT_PHY_CONTROL_PRBS_TX_ENABLE, (uint32 *)value);
        break;

    case bcmPortControlPrbsRxEnable:
        rv = bcmi_esw_portctrl_phy_control_get(unit, port, 
             BCM_ESW_PORTCTRL_PHYCTRL_PRBS_RX_ENABLE, (uint32 *)value);
        break;

    case bcmPortControlPrbsTxInvertData:
        rv = bcmi_esw_portctrl_phy_control_get(unit, port, 
             BCM_PORT_PHY_CONTROL_PRBS_TX_INVERT_DATA, (uint32 *)value);
        break;

    case bcmPortControlPrbsRxStatus:
        rv = bcmi_esw_portctrl_phy_control_get(unit, port, 
             BCM_PORT_PHY_CONTROL_PRBS_RX_STATUS, (uint32 *)value);
        break;

    case bcmPortControlLinkFaultLocalEnable:
        {
            portmod_local_fault_control_t control;

            PORT_LOCK(unit);
            rv = portmod_port_local_fault_control_get(unit, pport, &control);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = control.enable;
            }
        }
        break;
    case bcmPortControlLinkFaultRemoteEnable:
        {
            portmod_remote_fault_control_t control;

            PORT_LOCK(unit);
            rv = portmod_port_remote_fault_control_get(unit, pport, &control);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = control.enable;
            }
        }
        break;

    case bcmPortControlLinkFaultLocal:
        PORT_LOCK(unit);
        rv = portmod_port_local_fault_status_get(unit, pport, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlLinkFaultRemote:
        PORT_LOCK(unit);
        rv = portmod_port_remote_fault_status_get(unit, pport, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlTimestampTransmit:
        if (soc_feature(unit, soc_feature_timesync_support)) {
            portmod_fifo_status_t info;

            PORT_LOCK(unit);
            rv = portmod_port_diag_fifo_status_get(unit, pport, &info);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = info.timestamps_in_fifo;
            }
        }

        break;

    case bcmPortControlSerdesDriverEqualizationTuneStatusFarEnd:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           SOC_PHY_CONTROL_SERDES_DRIVER_EQUALIZATION_TUNE_STATUS_FAR_END,
           (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlLinkdownTransmit:
        PORT_LOCK(unit);
        rv = bcmi_esw_portctrl_phy_control_get(unit, port,
                 BCM_PORT_PHY_CONTROL_LINKDOWN_TRANSMIT, (uint32 *)value);
        PORT_UNLOCK(unit);
        if (rv == BCM_E_NONE) {
            rv = _bcm_esw_link_down_tx_get(unit, port, value);
        }
        break;

    case bcmPortControlSerdesTuneMarginMode:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE,
           (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlSerdesTuneMarginValue:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE,
           (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlSerdesTuneMarginMax:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MAX,
           (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlEEEEnable:
        
#if 0
        if (soc_feature(unit, soc_feature_eee)) {
            *value = eee_cfg[unit][port];
            rv = BCM_E_NONE;
        }
#endif
        break;

 /**********************      EEE Mode Overview    ************************
 *                      |DET|                              |  WT |
 *   Signalling   |idles|   |------------------------------|     | idles   |
 *   from Tx MAC  | or  |   |   Low Power Idle (LPI)       |idles|  or     |
 *   to local PHY |data |   |------------------------------|     | data    |
 *                          *                              *
 *                          *                              *
 *                          *  -------LPI state------------*
 *   Local PHY    |         |  |      |  |       |  |      |   |           |
 *   signaling    |   Active|Ts|  Tq  |Tr|  Tq   |Tr|  Tq  |Tw |Active     |
 *   on MDI       |         |  |      |  |       |  |      |   |           |
 *                          *------------------------------*
 *                          *                              *
 *                          *                               *
 *   Signaling    |   idles |-------------------------------|id| PHY is    |
 *   from LP PHY  |     or  |  Low Power Idle (LPI)         |le| ready     |
 *   to Rx MAC    |   data  |-------------------------------|s | for data  |
 *
 *   where DET = Delay Entry Timer    WT = MAC Wake Timer
 */
    case bcmPortControlEEETransmitIdleTime:
        /* DET = Time (in microsecs) for which condition to move to LPI state
         * is satisfied, at the end of which MAC TX transitions to LPI state */
        if (soc_feature (unit, soc_feature_eee)) {
            portmod_eee_t eee;
            PORT_LOCK(unit);
            rv = portmod_port_eee_get(unit, pport, &eee);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = eee.tx_idle_time;
            }
        }

        break;

    case bcmPortControlEEETransmitEventCount:
        /* Number of time MAC TX enters LPI state for
         * a given measurement interval*/
        if (soc_feature(unit, soc_feature_eee)) {
            uint64 rval64;
            
            rv = READ_TX_EEE_LPI_EVENT_COUNTERr(unit, port, &rval64);
            if (SOC_SUCCESS(rv)) {
                *value = COMPILER_64_LO(rval64);
            }
        }
        break;

    case bcmPortControlEEETransmitDuration:
        /* Time in (microsecs) for which MAC TX enters LPI state
         * during a measurement interval*/
        if (soc_feature(unit, soc_feature_eee)) {
            uint64 rval64;

            rv = READ_TX_EEE_LPI_DURATION_COUNTERr(unit, port, &rval64);
            if (SOC_SUCCESS(rv)) {
                *value = COMPILER_64_LO(rval64);
            }
        }

        break;

    case bcmPortControlEEEReceiveEventCount:
        /* Number of time MAC RX enters LPI state for
         * a given measurement interval */
        if (soc_feature(unit, soc_feature_eee)) {
            uint64 rval64;

            rv = READ_RX_EEE_LPI_EVENT_COUNTERr(unit, port, &rval64);
            if (SOC_SUCCESS(rv)) {
                *value = COMPILER_64_LO(rval64);
            }
        }
        break;

    case bcmPortControlEEEReceiveDuration:
        /* Time in (microsecs) for which MAC RX enters LPI state
         * during a measurement interval*/
        if (soc_feature(unit, soc_feature_eee)) {
            uint64 rval64;

            rv = READ_RX_EEE_LPI_DURATION_COUNTERr(unit, port, &rval64);
            if (SOC_SUCCESS(rv)) {
                *value = COMPILER_64_LO(rval64);
            }
        }
        break;

    case bcmPortControlEEETransmitRefreshTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_REFRESH_TIME,
           (uint32 *)value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitSleepTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_SLEEP_TIME, (uint32 *)value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitQuietTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_QUIET_TIME, (uint32 *)value);
        */
        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitWakeTime:
        /* Time(in microsecs) to wait before transmitter can leave LPI State*/
        if (soc_feature(unit, soc_feature_eee)) {
            portmod_eee_t eee;

            PORT_LOCK(unit);
            rv = portmod_port_eee_get(unit, pport, &eee);
            PORT_UNLOCK(unit);
            if (PORTMOD_SUCCESS(rv)) {
                *value = eee.tx_wake_time;
            }
        }
        break;

    case bcmPortControlEEEReceiveSleepTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_RECEIVE_SLEEP_TIME, (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlEEEReceiveQuietTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_RECEIVE_QUIET_TIME, (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlEEEReceiveWakeTime:
        PORT_LOCK(unit);
        
        /* rv = soc_phyctrl_control_get(unit, port,
           BCM_PORT_PHY_CONTROL_EEE_RECEIVE_WAKE_TIME, (uint32 *)value);
        */
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlRxEnable:
        rv = bcmi_esw_portctrl_mac_rx_control(unit, port, 1, value);
        break;

    default:
        /* Control is not supported */
        break;
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *      bcmi_esw_portctrl_mode_get
 * Description:
 *      Get the current mode used for MAC
 * Parameters:
 *      unit(IN)  - Device number
 *      port(IN)  - Port number
 *      mode(OUT) - The configured MAC mode  
 * Return Value:
 *      BCM_E_XXX
 */
int
bcmi_esw_portctrl_mode_get(int unit, bcm_gport_t port, int *mode)
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    portmod_port_mode_info_t info;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    portmod_port_mode_info_t_init(unit, &info);

    PORT_LOCK(unit);

    rv = portmod_port_mode_get(unit, pport, &info);

    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        *mode = info.cur_mode;
    }

    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_e2e_tx_enable_get
 * Purpose:
 *      Get E2E Enable
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      enable - (OUT) Whether to enable or disable
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_e2e_tx_enable_get(int unit, bcm_gport_t port, int *enable)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_e2e_enable_get(unit, pport, enable);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_e2e_tx_enable_set
 * Purpose:
 *      Set E2E Tx Enable
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      enable - (IN) Whether to enable or disable
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_e2e_tx_enable_set(int unit, bcm_gport_t port, int enable)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_e2e_enable_set(unit, pport, enable);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_e2ecc_hdr_get
 * Purpose:
 *      Get E2ECC header data
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      e2ecc_hdr - (OUT) E2ECC header.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_e2ecc_hdr_get(int unit, bcm_gport_t port,
                               soc_higig_e2ecc_hdr_t *e2ecc_hdr)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;
    portmod_port_higig_e2ecc_hdr_t portmod_hdr;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    if (!e2ecc_hdr) {
        return BCM_E_PARAM;
    }

    portmod_port_higig_e2ecc_hdr_t_init(unit, &portmod_hdr);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_e2ecc_hdr_get(unit, pport, &portmod_hdr);
    PORT_UNLOCK(unit);

    if (PORTMOD_SUCCESS(rv)) {
        sal_memcpy(e2ecc_hdr, &portmod_hdr, sizeof(soc_higig_e2ecc_hdr_t));
    }

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_e2ecc_hdr_set
 * Purpose:
 *      Set E2ECC header data
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      e2ecc_hdr - (IN) E2ECC header.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_e2ecc_hdr_set(int unit, bcm_gport_t port,
                               soc_higig_e2ecc_hdr_t *e2ecc_hdr)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;
    portmod_port_higig_e2ecc_hdr_t portmod_hdr;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    if (!e2ecc_hdr) {
        return BCM_E_PARAM;
    }

    portmod_port_higig_e2ecc_hdr_t_init(unit, &portmod_hdr);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    sal_memcpy(&portmod_hdr, e2ecc_hdr, sizeof(portmod_port_higig_e2ecc_hdr_t));

    PORT_LOCK(unit);
    rv = portmod_port_e2ecc_hdr_set(unit, pport, &portmod_hdr);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_vlan_tpid_set
 * Purpose:
 *      Set TPID in vlan_tag.
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      tpid - (IN) tpid value.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_vlan_tpid_set(int unit, bcm_gport_t port, int tpid)
{
#ifdef PORTMOD_SUPPORT
    int                rv;
    portctrl_pport_t   pport;
    portmod_vlan_tag_t vlan_tag;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN(portmod_port_vlan_tag_get(unit, port, &vlan_tag));

    vlan_tag.outer_vlan_tag = tpid;
  
    PORT_LOCK(unit);
    rv = portmod_port_vlan_tag_set(unit, port, &vlan_tag);
    PORT_UNLOCK(unit);

    return (rv);
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_vlan_inner_tpid_set
 * Purpose:
 *      Set TPID in vlan_tag.
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      tpid - (IN) tpid value.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_vlan_inner_tpid_set(int unit, bcm_gport_t port, int tpid)
{
#ifdef PORTMOD_SUPPORT
    int                rv;
    portctrl_pport_t   pport;
    portmod_vlan_tag_t vlan_tag;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    BCM_IF_ERROR_RETURN(portmod_port_vlan_tag_get(unit, port, &vlan_tag));

    vlan_tag.inner_vlan_tag = tpid;
    
  
    PORT_LOCK(unit);
    rv = portmod_port_vlan_tag_set(unit, port, &vlan_tag);
    PORT_UNLOCK(unit);

    return (rv);
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_cntmaxsize_get
 * Purpose:
 *      Get the max packet size that is used in statistic counter update
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      val - (OUT) max packet size.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_cntmaxsize_get(int unit, bcm_gport_t port, int *val)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;

/* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_cntmaxsize_get(unit, pport, val);
    PORT_UNLOCK(unit);

    return rv;
#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}
/*
 * Function:
 *     bcmi_esw_portctrl_cntmaxsize_set
 * Purpose:
 *      Set the max packet size that is used in statistic counter update
 * Parameters:
 *      unit - (IN) SOC unit number.
 *      port - (IN) Port number.
 *      val - (IN) max packet size.
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_portctrl_cntmaxsize_set(int unit, bcm_gport_t port, int val)
{
#ifdef PORTMOD_SUPPORT
    int         rv;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN
        (PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    PORT_LOCK(unit);
    rv = portmod_port_cntmaxsize_set(unit, pport, val);
    PORT_UNLOCK(unit);

    return rv;

#else  /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* PORTMOD_SUPPORT */
}

/*
 * Function:
 *     bcmi_esw_portctrl_cable_diag
 * Description:
 *      Run Cable Diagnostics on port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      status - (OUT) cable diag status structure
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Cable diagnostics are only supported by some phy types
 *      (currently 5248 10/100 phy and 546x 10/100/1000 phys)
 */
 int
 bcmi_esw_portctrl_cable_diag(int unit, bcm_port_t port,
                    bcm_port_cable_diag_t *status) 
{
#ifdef PORTMOD_SUPPORT
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;
    int ext_phy_present;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    /* PORTMOD currently only supports medium config get for ports with legacy ext PHY */
    SOC_IF_ERROR_RETURN(portmod_port_is_legacy_ext_phy_present(unit, pport, &ext_phy_present));
    if (!ext_phy_present) { 
        return BCM_E_UNAVAIL; 
    }

    PORT_LOCK(unit);
    rv = soc_phyctrl_cable_diag(unit, port, status);
    PORT_UNLOCK(unit);
    
    return rv;

#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}

#if defined(INCLUDE_PHY_542XX)
/*
 * Function:
 *     bcmi_esw_portctrl_serdes_link_update
 * Description:
 *     Update the force link status of Serdes
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      link - Link Status of the port
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 */
 int
 bcmi_esw_portctrl_serdes_link_update(int unit, bcm_port_t port, int link)
{
#ifdef PORTMOD_SUPPORT
    portmod_port_update_control_t portmod_port_update_control;
    int rv = BCM_E_NONE;
    portctrl_pport_t pport;

    /* Make sure port module is initialized. */
    PORTCTRL_INIT_CHECK(unit);
    BCM_IF_ERROR_RETURN(PORTCTRL_PORT_RESOLVE(unit, port, &port, &pport));

    SOC_IF_ERROR_RETURN(portmod_port_update_control_t_init(unit,
                                           &portmod_port_update_control));
    portmod_port_update_control.link_status = link;
    PORTMOD_PORT_UPDATE_F_UPDATE_SERDES_LINK_SET(&portmod_port_update_control);
    PORT_LOCK(unit);
    rv = portmod_port_update(unit, port, &portmod_port_update_control);
    PORT_UNLOCK(unit);
    return rv;
#else   /* PORTMOD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif  /* PORTMOD_SUPPORT */
}
#endif /* defined(INCLUDE_PHY_542XX) */
