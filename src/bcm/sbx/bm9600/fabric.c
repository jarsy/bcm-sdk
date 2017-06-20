/*
 * $Id:$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 Fabric Control API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/port.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>


#define BM9600_NS_PER_CLOCK (4) /* 250MHz clock, 4 ns, per clock */

#define PLANE_A 0
#define PLANE_B 1
#define PLANE_BOTH 2

int
bcm_bm9600_fabric_crossbar_connection_set(int unit,
                                          int xbar, 
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t dst_xbport)
{
    int rv;
    int dst_node, dst_phy_node;
    int src_node;
    bcm_port_t dst_port, src_port;
    int configure_src_port = TRUE, configure_src_portb = FALSE;
    bcm_port_t dst_portb = 0, src_portb = 0;
    int src_dual_channel_even = TRUE;
    int dst_dual_channel_even = TRUE;
    int arbiter_id, plane= PLANE_BOTH;
    static int first_call = TRUE;
    uint32 unused_plane_remap = 0;
    
    /* The bcm_port_t is a physical port, which is split into two logical ports. We map both
     * logical ports to the equivalent logical ports on the other side of the crossbar.
     */
    if (!BCM_STK_MOD_IS_NODE(dst_modid)) {
        return BCM_E_PARAM;
    }
    dst_node = BCM_STK_MOD_TO_NODE(dst_modid);
    dst_phy_node = SOC_SBX_L2P_NODE(unit, dst_node);
    
    if (!BCM_STK_MOD_IS_NODE(src_modid)) {
        return(BCM_E_PARAM);
    }
    src_node = BCM_STK_MOD_TO_NODE(src_modid);
    
    /* Sequencing required for sfi_local channels, need to use 2 continguous lxbars   */
    /* so need to call back to back for A-A channels.  State is reserved in this case */
    if (first_call == FALSE) {
        if ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_DUAL_SFI) &&
            (SOC_SBX_STATE(unit)->stack_state->protocol[src_node] == bcmModuleProtocol4)) {
            src_dual_channel_even = FALSE;
        }
        if ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[dst_xbport] == BCM_PORT_ABILITY_DUAL_SFI) &&
            (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node] == bcmModuleProtocol4)) {
            dst_dual_channel_even = FALSE;
        }
        first_call = TRUE;
    } else { /* First call=true */
        first_call = FALSE;
    }
    
    /* bcmModuleProtocol1: (qe2000, plane A)
     *    assuming using even xbar ports
     * bcmModuleProtocol2: (qe2000, plane B)
     *    assuming using odd xbar ports  
     * bcmModuleProtocol3: (sirius, plane A/B)
     *    assuming using both even/odd xbar ports, even connect to even, odd connect to odd
     * bcmModuleProtocol4: (sirius, plane A B local)
     *    assuming using both even/odd xbar ports
     * bcmModuleProtocol5: (sirius plane A for 3.125G single sfi)
     */    
    
    /* Need to set xbar for correct plane if SFI setup for SFI_SCI link */
    /* By default, Arbiter 0 is plane A and arbiter 1 is plane B        */
    if ( (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) ||
         (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[dst_xbport] == BCM_PORT_ABILITY_SFI_SCI) ) {
        
        /* consistency check.                                                            */
        /* Currently supports the following connection                                   */
        /*   - Both ends should have identical ability (i.e. one SCI channel and other   */
        /*     SFI channel. Thus support across identical devices.                       */
        /*   - Planes hidden from Application. SDK convention of choosing planes based   */
        /*     of arbiter. Thus identical bandwidth across both planes (when there is    */
        /*     no failure)                                                               */
        /*                                                                               */
        /* Curently the following connection not supported                               */
        /*   - One end being {SCI, SFI} (e.g. sirius) and the other end being {SFI}      */
        /*     (e.g qe2000). Error on SCI of sirius device) effects even SFI links of    */
        /*     QE2000 device. Ideally would prfer a connection where SCI errors do not   */
        /*     effect QE2000 SFI links and a configuration where sirius bandwidth is     */
        /*     same across both planes                                                   */
        /*     However the above is valid and will be supported when the use case is     */
        /*     needed.                                                                   */
        if (!((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) &&
              (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[dst_xbport] == BCM_PORT_ABILITY_SFI_SCI))) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: src/dst xbport abilities not supported, Unit: %d srcNode(%d), srcPort(%d), srcPortAbility(%d) dstNode(%d), dstPort(%d), dstPortAbility(%d)\n"),
                       unit,
                       src_node, src_xbport, SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport],
                       dst_node, dst_xbport, SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[dst_xbport]));
            return(BCM_E_PARAM);
        }

        rv = bcm_fabric_control_get(unit, bcmFabricArbiterId, &arbiter_id);
        COMPILER_REFERENCE(rv);
        if (arbiter_id == 0) {
            plane = PLANE_A;
        }else {
            plane = PLANE_B;
        }
        /* operate on the odd channel, there are not 2 logical                           */
        /* crossbars for the sfi_sci link, only 1                                        */
        src_dual_channel_even = FALSE;
        dst_dual_channel_even = FALSE;
        
        configure_src_port = FALSE;
        configure_src_portb = TRUE;
    }
    switch (SOC_SBX_STATE(unit)->stack_state->protocol[src_node]) {
    case bcmModuleProtocol1:
    case bcmModuleProtocol5:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol5:
		    src_port = src_xbport * 2;
            /* Populate both plane entries as xcfg tx_plane may be set to 1 */
            dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
		    break;
		case bcmModuleProtocol2:
		    src_port = (src_xbport * 2);
		    dst_port = (dst_xbport * 2) + 1;
		    break;
		case bcmModuleProtocol3:
		    src_port = src_xbport * 2;
		    dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    break;
		case bcmModuleProtocol4:
		    /* sequencing, first even, then odd */
		    src_port = src_xbport * 2;
            
            if (dst_dual_channel_even) {
                dst_port = dst_xbport * 2;
            } else {
                dst_port = dst_xbport * 2 + 1;
            }
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
        
    case bcmModuleProtocol2:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol5:
            src_port = (src_xbport * 2) + 1;
            dst_port = (dst_xbport * 2);
            break;
        case bcmModuleProtocol2:
            src_port = (src_xbport * 2) + 1;
            dst_port = (dst_xbport * 2) + 1;
            break;
        case bcmModuleProtocol3:
            src_port = (src_xbport * 2) + 1;
            dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            break;
        case bcmModuleProtocol4:
            /* sequencing, first even, then odd */
            src_port = src_xbport * 2 + 1;
            
            if (dst_dual_channel_even) {
                dst_port = dst_xbport * 2;
            } else {
                dst_port = dst_xbport * 2 + 1;
            }
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
    case bcmModuleProtocol3:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol5:
            src_port = src_xbport * 2;
            dst_port  = (dst_xbport * 2) | ((dst_xbport * 2 & 0xFF) << 8);
            
            configure_src_portb = TRUE;
            src_portb = src_xbport * 2 + 1;
            dst_portb = (dst_xbport * 2) | ((dst_xbport * 2 & 0xFF) << 8);
            break;
        case bcmModuleProtocol2:
            src_port = (src_xbport * 2);
            dst_port = (dst_xbport * 2) + 1;
            
            configure_src_portb = TRUE;
            src_portb = (src_xbport * 2) + 1;
            dst_portb = (dst_xbport * 2) + 1;
            break;
        case bcmModuleProtocol3:
            src_port = src_xbport * 2;                        
            configure_src_portb = TRUE;
            src_portb = (src_xbport * 2) + 1;

            if (plane == PLANE_BOTH) {
                dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
                dst_portb = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            } else {
                /* For VPORT_MIX, we need to set xcfg value to an unused iport to avoid
                 * crossover plane FR errors on Sirius.  For VPORT customers, we don't want to
                 * change their working behavior - which is setting the unused planet to 0.
                 */
                if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
                    unused_plane_remap = (SOC_SBX_CFG_BM9600(unit)->arbiterUnusedLinkForSfiSciXcfgRemap * 2) + 1;
                } else {
                    unused_plane_remap = 0xff;
                }                
                if (plane == PLANE_A){
                    /* even channel never used SCI */
                    dst_port  = (unused_plane_remap | (unused_plane_remap << 8));
                    /* odd channel */
                    dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (unused_plane_remap << 8);
                }else{
                    /* even channel never used SCI */
                    dst_port  = (unused_plane_remap | (unused_plane_remap << 8));
                    /* odd channel */
                    dst_portb = (unused_plane_remap | (((dst_xbport * 2 + 1) & 0xFF) << 8));
                }
            }
            break;
            
        case bcmModuleProtocol4:
            /* sequencing, first even, then odd */
            src_port = src_xbport * 2;
            
            if (dst_dual_channel_even) {
                dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
            } else {
                dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            }
            
            configure_src_portb = TRUE;
            src_portb = src_xbport * 2 + 1;
            
            if (dst_dual_channel_even) {
                dst_portb = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8); 
            } else {
                dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            }
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
    case bcmModuleProtocol4:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol5:
            if (src_dual_channel_even) {
                src_port = src_xbport * 2;
            } else {
                src_port = src_xbport * 2 + 1;
            }
            
            dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
            break;
        case bcmModuleProtocol2:
            if (src_dual_channel_even) {
                src_port = src_xbport * 2;
            } else {
                src_port = src_xbport * 2 + 1;
            }
            
            dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            break;
        case bcmModuleProtocol3:
            src_port = src_xbport * 2;
            
            if (dst_dual_channel_even) {
                dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
            } else {
                dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            }
            
            /* Set up XCFG for odd channel (plane B) just in case */
            configure_src_portb = TRUE;
            src_portb = (src_xbport * 2) + 1;
            if (dst_dual_channel_even) {
                dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
            } else {
                dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            }
            break;
        case bcmModuleProtocol4:            
            src_port = src_xbport * 2;
            dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
            
            configure_src_portb = TRUE;
            src_portb = (src_xbport * 2) + 1;
            dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
        
        case bcmModuleProtocolCustom1:
            switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
            case bcmModuleProtocolCustom1:
                src_port = src_xbport * 2;
                dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
                
                configure_src_portb = TRUE;
                src_portb = (src_xbport * 2) + 1;
                if (plane == PLANE_BOTH) {
                        dst_portb = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
                }
                    else {
                        /* This is an SFI_SCI link, odd channel can be plane A or B, but always destination odd entry set up */
                        dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);

                        if (plane == PLANE_A) {
                            dst_port |= (0xff<<8); /* planeB not used - must point to 0xff */
                            dst_portb |= (0xff<<8);
                        }
                        else {
                            dst_port |= 0xff; /* planeA not used - must point to 0xff */
                            dst_portb |= 0xff; /* planeA not used - must point to 0xff */
                        }
                    }
                break;
                
            default:
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                           unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                           SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
            }
            
            break;
            
    default:
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Unit %d, unsupported srcNodeProtocol: 0x%x)\n"),
                   unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node]));
        return(BCM_E_PARAM);
    }
    
    if (configure_src_port == TRUE) {
        rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, dst_phy_node, src_port, dst_port);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d, addr: %d select: %d value: 0x%x\n"),
                       src_xbport, dst_phy_node, src_port, dst_port));
            return(BCM_E_FAIL);
        }
    }
    
    if (configure_src_portb == TRUE) {
        rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, dst_phy_node, src_portb, dst_portb);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d, addr: %d select: %d value: 0x%x\n"),
                       src_xbport, dst_phy_node, src_portb, dst_portb));
            return(BCM_E_FAIL);
         }
    }
    
    return(BCM_E_NONE);
}

int
bcm_bm9600_fabric_crossbar_connection_get(int unit,
                                          int xbar, 
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    int rv;
    int dst_node, dst_phy_node;
    int src_node;
    bcm_port_t src_port, src_portb;
    int src_dual_channel_even = TRUE;
    static int first_call = TRUE;
    int arbiter_id, plane= PLANE_BOTH;
    int configure_src_port = TRUE, configure_src_portb = FALSE;

    src_portb = 0;

    if (!BCM_STK_MOD_IS_NODE(dst_modid)) {
        return BCM_E_PARAM;
    }
    dst_node = BCM_STK_MOD_TO_NODE(dst_modid);
    dst_phy_node = SOC_SBX_L2P_NODE(unit, dst_node);
    
    if (!BCM_STK_MOD_IS_NODE(src_modid)) {
        return(BCM_E_PARAM);
    }
    src_node = BCM_STK_MOD_TO_NODE(src_modid);
    
    /* Sequencing required for sfi_local channels, need to use 2 continguous lxbars   */
    /* so need to call back to back for A-A channels.  State is reserved in this case */
    if (first_call == FALSE) {
        if ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_DUAL_SFI) &&
        (SOC_SBX_STATE(unit)->stack_state->protocol[src_node] == bcmModuleProtocol4)) {
            src_dual_channel_even = FALSE;
        }
        first_call = TRUE;
    } else { /* First call=true */
        first_call = FALSE;
    }
    
    if (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) {
        
        rv = bcm_fabric_control_get(unit, bcmFabricArbiterId, &arbiter_id);
        COMPILER_REFERENCE(rv);
        if (arbiter_id == 0) {
            plane = PLANE_A;
        }else {
            plane = PLANE_B;
        }
        /* And if protocol 4, always operate on the odd channel, there are not 2 logical crossbars for the sfi_sci link, only 1 */
        src_dual_channel_even = FALSE;
        
        configure_src_port = FALSE;
        configure_src_portb = TRUE;
    }
    
    switch (SOC_SBX_STATE(unit)->stack_state->protocol[src_node]) {
    case bcmModuleProtocol1:
    case bcmModuleProtocol5:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol2:
        case bcmModuleProtocol3:
        case bcmModuleProtocol4:
        case bcmModuleProtocol5:
            src_port = src_xbport * 2;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
    case bcmModuleProtocol2:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol2:
        case bcmModuleProtocol3:
        case bcmModuleProtocol5:
        case bcmModuleProtocol4:
            src_port = (src_xbport * 2) + 1;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
    case bcmModuleProtocol3:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol5:
        case bcmModuleProtocol2:
        case bcmModuleProtocol3:
        case bcmModuleProtocol4:
            configure_src_portb = TRUE;
            src_port = src_xbport * 2;
            src_portb = src_xbport * 2 + 1;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
        
    case bcmModuleProtocol4:
        switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol2:
        case bcmModuleProtocol5:
            if (src_dual_channel_even) {
                src_port = src_xbport * 2;
            } else {
                src_port = src_xbport * 2 + 1;
            }
            break;
        case bcmModuleProtocol3:
        case bcmModuleProtocol4:
            src_port = src_xbport * 2;
            configure_src_portb = TRUE;
            src_portb = (src_xbport * 2) + 1;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n"),
                       unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
                       SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
            return(BCM_E_PARAM);
        }
        break;
        
    case bcmModuleProtocolCustom1:
        src_port = src_xbport * 2;
        src_portb = (src_xbport * 2) + 1;
        configure_src_portb = TRUE;
        break;
        
    default:
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Unit %d, unsupported srcNodeProtocol: 0x%x)\n"),
                   unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node]));
        return(BCM_E_PARAM);
    }
    
    if (configure_src_port == TRUE) {
        /* Read the first source */
        rv = soc_bm9600_XbXcfgRemapSelectRead(unit, dst_phy_node, src_port, (uint32*)dst_xbport);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n"), src_xbport));
            return BCM_E_FAIL;
        }
    }
    /* Since only 1 entry can be returned, always return B value if more than one entry */
    if (configure_src_portb == TRUE) {
        rv = soc_bm9600_XbXcfgRemapSelectRead(unit, dst_phy_node, src_portb, (uint32*)dst_xbport);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n"), src_xbport));
            return BCM_E_FAIL;
        }
    }
    else {
        return(BCM_E_PARAM);
    }
    
    if (plane == PLANE_B) {
        (*dst_xbport) = ((*dst_xbport) & (0xFF << 8)) >> 8;
    } else {
        (*dst_xbport) = (*dst_xbport) & 0xFF;
    }
    if ((*dst_xbport) & 0x1) {
        (*dst_xbport) &= ~(0x1);
    }
    *dst_xbport /= 2;
    
    /* consistency check */
    if (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) {
        /* consistency check.                                                            */
        if (!((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) &&
              (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[(*dst_xbport)] == BCM_PORT_ABILITY_SFI_SCI))) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: src/dst xbport abilities not supported, Unit: %d srcNode(%d), srcPort(%d), srcPortAbility(%d) dstNode(%d), dstPort(%d), dstPortAbility(%d)\n"),
                       unit,
                       src_modid, src_xbport, SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport],
                       dst_modid, (*dst_xbport), SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[(*dst_xbport)]));
            return(BCM_E_PARAM);
        }
    }

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_calendar_active(int unit)
{
    int rv = BCM_E_NONE;
    uint32 uData;

    /* Swap active calendar */
    uData = SAND_HAL_READ(unit, PL, BW_GROUP_CONFIG);
    if (uData & 1) {
        uData = uData & 0xffffffe;
    }else {
        uData = uData | 1;
    }
    SAND_HAL_WRITE(unit, PL, BW_GROUP_CONFIG, uData);
    return rv;
}

int
bcm_bm9600_fabric_crossbar_mapping_set(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id, 
                                       int xbar,
                                       bcm_port_t port)
{
   int rv = BCM_E_NONE;
    int node = 0, nt1 = FALSE;
    bcm_module_protocol_t protocol;
    int force_link_active = FALSE;
    uint32 regval = 0;
    int lxbar, lxbar1;
    int sfi_port;

    /* Port passed in from common sbx has been adjusted to base sfi port */
    sfi_port = port;

    node = BCM_STK_MOD_TO_NODE(modid);
    
    rv = bcm_sbx_stk_module_protocol_get(unit, modid, &protocol);
    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: module protocol not set up for modid(%d)\n"),
                   modid));
        return rv;
    }
    
    /* unmap */
    if (xbar == -1) {
        rv = bcm_sbx_fabric_xbar_for_sfi_get(unit, node, sfi_port, &lxbar, &lxbar1);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unmap failure port(%d)\n"),
                       sfi_port));
            return rv;
        }
        if (BCM_SBX_CROSSBAR_VALID(lxbar)) {
            rv = bcm_sbx_fabric_nodetype_for_xbar_set(unit, lxbar, FALSE /* not qe2k */);
            if (rv) {
                return rv;
            }
            
            regval = SAND_HAL_READ(unit, PL, FO_CONFIG14);
#ifdef DEBUG_CROSSBAR_MAP
            LOG_CLI((BSL_META_U(unit,
                                "%s: unmap fabric crossbar(%d) associated with port(%d)\n"), 
                     FUNCTION_NAME(), lxbar, sfi_port));
#endif 
            /* coverity[large_shift] */
            regval &= ~(1 << lxbar);
            
            SAND_HAL_WRITE(unit, PL, FO_CONFIG14, regval);
            
        }

        if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
            rv = bcm_sbx_fabric_nodetype_for_xbar_set(unit, lxbar1, FALSE /* not qe2k */);
            if (rv) {
                return rv;
            }
            regval = SAND_HAL_READ(unit, PL, FO_CONFIG14);
#ifdef DEBUG_CROSSBAR_MAP
            LOG_CLI((BSL_META_U(unit,
                                "%s: unmap fabric crossbar(%d) associated with port(%d)\n"), 
                     FUNCTION_NAME(), lxbar, sfi_port));
#endif 
            /* coverity[large_shift] */
            regval &= ~(1 << lxbar);
            
            SAND_HAL_WRITE(unit, PL, FO_CONFIG14, regval);
            
        }
        
    } else {

        /* Set up Force Link Active */
        switch (protocol) {
        case bcmModuleProtocol1:
        case bcmModuleProtocol2:
            rv = bcm_sbx_fabric_nodetype_for_xbar_set(unit, xbar, TRUE/* qe2k */);
            if (rv) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: map failure node(%d) xbar(%d) qe2k(%s)\n"),
                           node, xbar, "TRUE"));
                return rv;
            }
            break;
        case bcmModuleProtocol3:
        case bcmModuleProtocol5:
        case bcmModuleProtocol4:
            /* nothing to do */
            break;
        default:
            rv = BCM_E_CONFIG;
            return rv;
            break;
        }
        
        rv = bcm_sbx_fabric_nodetype_for_xbar_get(unit, xbar, &nt1);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: map failure node(%d) xbar(%d) qe2k(%s)\n"),
                       node, xbar, nt1?"TRUE":"FALSE"));
            return rv;
        }

        /* set force link active on qe2k node type for crossbars which are ss only  */
        if (nt1 == FALSE) {
            force_link_active = TRUE;
        } else {
            force_link_active = FALSE;
        }

        /* corresponds to node_type 1 QE2k */
        regval = SAND_HAL_READ(unit, PL, FO_CONFIG14);
        if (force_link_active == TRUE) {
#ifdef DEBUG_CROSSBAR_MAP
            LOG_CLI((BSL_META_U(unit,
                                "%s: node(%d) xbar(%d) FORCE ACTIVE\n"), FUNCTION_NAME(), node, xbar));
#endif 
            regval |= (1 << xbar);
        } else {
#ifdef DEBUG_CROSSBAR_MAP
            LOG_CLI((BSL_META_U(unit,
                                "%s: node(%d) xbar(%d) not FORCED\n"), FUNCTION_NAME(), node, xbar));
#endif 
            regval &= ~(1 << xbar);
        }
        SAND_HAL_WRITE(unit, PL, FO_CONFIG14, regval);

    } 
    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_crossbar_mapping_get(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id, 
                                       int xbar,
                                       bcm_port_t *port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_bm9600_fabric_crossbar_enable_set(int unit,
                                      uint64 xbars)
{
    uint32 i;
    int32  nTsSizeNormNs;
    int32  nSfiCount;
    int32  nHalfBus;
    int32  nOldTsSizeNormNs;
    int32  bw_group;
    uint32 num_queues_in_bag, base_queue, bag_rate_bytes_per_epoch;
    int32  nOldDemandScale, nDemandScale;
    uint32 gamma = 0, sigma = 0, new_sigma = 0;
    int32  queue;
    int    rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate_start;
    uint64 uu_epoch_length_in_ns = COMPILER_64_INIT(0,0);
    uint64 uu_bag_rate_in_bytes_per_sec;
    int32 bag_rate_kbps;
    uint32 new_bag_rate_bytes_per_epoch = 0;
    uint32 guarantee_in_kbps = 0;
    uint64 uu_guarantee_in_bytes_per_sec;
    uint32 guarantee_in_bytes_per_epoch = 0;

    
    nHalfBus = SOC_SBX_CFG(unit)->bHalfBus;
    /* set the link enables on the BME */
    nSfiCount = 0;
    for (i=0; i<64; i++) {
        /* Count number of enabled links */
        nSfiCount += ((COMPILER_64_LO(xbars) >> i) & 0x1);
    }

    nTsSizeNormNs = soc_sbx_fabric_get_timeslot_size(unit, nSfiCount, nHalfBus, soc_feature(unit, soc_feature_hybrid) );

    nOldTsSizeNormNs = SOC_SBX_STATE(unit)->fabric_state->timeslot_size;
    SOC_SBX_STATE(unit)->fabric_state->timeslot_size = nTsSizeNormNs;

    /* Force null grants */
    SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG0, FORCE_NULL_GRANT, 1);
    /* configure the fabric data plane */
    SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG3, LINK_ENABLE, COMPILER_64_LO(xbars));

    sal_usleep(10);

    /* And re-enable */
    SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG0, FORCE_NULL_GRANT, 0);

    /* reconfigure bags */
    nOldDemandScale = SOC_SBX_STATE(unit)->fabric_state->old_demand_scale;
    nDemandScale = SOC_SBX_CFG(unit)->demand_scale;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Unit(%d), DemandScale: %d\n"),
              FUNCTION_NAME(), unit,
              SOC_SBX_CFG(unit)->demand_scale));

    if ( (nOldTsSizeNormNs != nTsSizeNormNs) || (nOldDemandScale != nDemandScale) ) {
    /* go through all bags and reconfig all non-zero bag rate */
    p_bwstate_start = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;

        COMPILER_64_SET(uu_epoch_length_in_ns, 0, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
        COMPILER_64_UMUL_32(uu_epoch_length_in_ns, nTsSizeNormNs);
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    
    for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {
        p_bwstate = &p_bwstate_start[bw_group];
        
        if ( ( p_bwstate->in_use == FALSE ) || (p_bwstate->path.bag_rate_kbps == 0) ) {
        continue;
        }

            bag_rate_kbps = p_bwstate->path.bag_rate_kbps;
            uu_bag_rate_in_bytes_per_sec = uu_epoch_length_in_ns;
            COMPILER_64_UMUL_32(uu_bag_rate_in_bytes_per_sec, (bag_rate_kbps / 8));
            if (soc_sbx_div64(uu_bag_rate_in_bytes_per_sec, 1000000, &new_bag_rate_bytes_per_epoch) == -1) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: update BAG rate per epoch failed\n")));
                return(BCM_E_INTERNAL);
            }
            new_bag_rate_bytes_per_epoch >>= nDemandScale;

        rv = soc_bm9600_bag_read(unit, bw_group, &num_queues_in_bag,
                     &base_queue, &bag_rate_bytes_per_epoch);
        if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: BM9600 read of PRT table failed for bw_group(%d)\n"),
                   bw_group));
        return(BCM_E_FAIL);
        }

        rv = soc_bm9600_bag_write(unit, bw_group, num_queues_in_bag,
                      base_queue, new_bag_rate_bytes_per_epoch);
        if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: BM9600 Write to PRT table failed for bw_group(%d)\n"),
                   bw_group));
        return(BCM_E_FAIL);
        }

            base_queue = p_bwstate->base_queue;
            for (queue = base_queue; queue < (base_queue + p_bwstate->num_cos); queue++) {
                new_sigma = 0;
        rv = soc_bm9600_bwp_read(unit, queue, &gamma, &sigma);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: BM9600 Read to BWP table failed for queue(%d)\n"),
                       queue));
            return(BCM_E_FAIL);
        }

                if (!( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_SP) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_AF) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_EF) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP0) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP1) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP3) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP4) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP5) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP6) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP7) )) {
                    continue;
                }

                if (p_qstate[queue].ingress.bw_mode == BCM_COSQ_SP) {
                    new_sigma = (int32) new_bag_rate_bytes_per_epoch;
                }

                if ( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP0) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP1) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP2) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP3) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP4) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP5) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP6) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP7) ) {
                    new_sigma = (int32) new_bag_rate_bytes_per_epoch;
                }

                if ( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_AF) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_EF) ) {

                    guarantee_in_kbps = bcm_sbx_cosq_get_bw_guarantee(unit, queue);
                    uu_guarantee_in_bytes_per_sec = uu_epoch_length_in_ns;
                    COMPILER_64_UMUL_32(uu_guarantee_in_bytes_per_sec, (guarantee_in_kbps / 8));

                    if (soc_sbx_div64(uu_guarantee_in_bytes_per_sec, 1000000, &guarantee_in_bytes_per_epoch) == -1) {
                      LOG_ERROR(BSL_LS_BCM_COMMON,
                                (BSL_META_U(unit,
                                            "ERROR: update Guarantee failed\n")));
                      return(BCM_E_INTERNAL);
                    }
                    guarantee_in_bytes_per_epoch >>= nDemandScale;

                    new_sigma = (int32) guarantee_in_bytes_per_epoch;

                    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                (BSL_META_U(unit,
                                            "queue(%d) guarantee in bytes/epoch(%d) guarantee_in_bytes_per_msec(0x%x%08x) epoch_length_in_ns(0x%x%08x)\n"),
                                 queue, guarantee_in_bytes_per_epoch, 
                                 COMPILER_64_HI(uu_guarantee_in_bytes_per_sec), COMPILER_64_LO(uu_guarantee_in_bytes_per_sec),
                                 COMPILER_64_HI(uu_epoch_length_in_ns),COMPILER_64_LO(uu_epoch_length_in_ns) ));
                }

        rv = soc_bm9600_bwp_write(unit, queue, gamma, new_sigma);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: BM9600 Write to BWP table failed for queue(%d)\n"),
                       queue));
            return(BCM_E_FAIL);
        }
            }
    }
    }

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_crossbar_enable_get(int unit,
                                      uint64 *xbars)
{
    uint32 uData;

    uData = SAND_HAL_READ(unit, PL, FO_CONFIG3);
    COMPILER_64_SET(*xbars,0, SAND_HAL_GET_FIELD(PL, FO_CONFIG3, LINK_ENABLE, uData));

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_crossbar_status_get(int unit,
                                      uint64 *xbars)
{
    uint32 uData;

    uData = SAND_HAL_READ(unit, PL, FO_STATUS1);
    COMPILER_64_SET(*xbars,0,SAND_HAL_GET_FIELD(PL, FO_STATUS1, GLOBAL_ENABLED_LINKS, uData));

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_distribution_create(int unit,
                                      bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_UNAVAIL;


    return rv;
}

int
bcm_bm9600_fabric_distribution_destroy(int unit,
                                       bcm_fabric_distribution_t  ds_id)
{
    int rv = BCM_E_UNAVAIL;
    bcm_fabric_distribution_t nef_ds_id;
    int32 ds_id_region, cos_ds_id, fcd;
    uint64 uuZero = COMPILER_64_INIT(0,0);


    if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
        rv = soc_bm9600_eset_set(unit, ds_id, uuZero, 0, BCM_FABRIC_DISTRIBUTION_SCHED_ALL, 0);
        if (rv != SOC_E_NONE) {
            return(rv);
        }

        if (soc_bm9600_features(unit, soc_feature_egr_multicast_independent_fc)) {
            nef_ds_id = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, ds_id);
            rv = soc_bm9600_eset_set(unit, nef_ds_id, uuZero, 0, BCM_FABRIC_DISTRIBUTION_SCHED_ALL, 1);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
        }
    }
    else {
        for (ds_id_region = 0; ds_id_region < SOC_SBX_CFG(unit)->num_res_per_eset_spec;
                                                                            ds_id_region++) {
            rv = bcm_sbx_cosq_eset_to_cos_eset_fcd(unit, ds_id, ds_id_region, &cos_ds_id, &fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            rv = soc_bm9600_eset_set(unit, cos_ds_id, uuZero, 0, BCM_FABRIC_DISTRIBUTION_SCHED_ALL, fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
        }
    }

    return rv;
}

int
bcm_bm9600_fabric_distribution_set(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int modid_count,
                                   int *dist_modids,
                   int mc_full_eval_min)
{
    int rv = BCM_E_UNAVAIL;
    int i;
    uint64 low_eset_value =  COMPILER_64_INIT(0,0);
    uint32 hi_eset_value;
    int    node;
    uint64 mask = COMPILER_64_INIT(0,1);
    bcm_fabric_distribution_t nef_ds_id;
    int32 ds_id_region, cos_ds_id, fcd;


    for (i = 0, hi_eset_value = 0; i < modid_count; i++) {
        node = BCM_STK_MOD_TO_NODE((*(dist_modids + i)));
        if (node <= SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD) {
            COMPILER_64_SHL(mask, node);
            COMPILER_64_OR(low_eset_value, mask);
        }
        else {
            hi_eset_value |= (1 << (node - SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD - 1));
        }
    }

    if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
        rv = soc_bm9600_eset_set(unit, ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 0);
        if (rv != BCM_E_NONE) {
            return(rv);
        }

        if (soc_bm9600_features(unit, soc_feature_egr_multicast_independent_fc)) {
            nef_ds_id = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, ds_id);
            rv = soc_bm9600_eset_set(unit, nef_ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 1);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
        }
    }
    else {
        for (ds_id_region = 0; ds_id_region < SOC_SBX_CFG(unit)->num_res_per_eset_spec;
                                                                            ds_id_region++) {
            rv = bcm_sbx_cosq_eset_to_cos_eset_fcd(unit, ds_id, ds_id_region, &cos_ds_id, &fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            rv = soc_bm9600_eset_set(unit, cos_ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
        }
    }

    return rv;
}

int
bcm_bm9600_fabric_distribution_get(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int max_count,
                                   int *dist_modids,
                                   int *count)
{
    int rv = BCM_E_UNAVAIL;
    int node, num_members;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    uint32 mc_full_eval_min;
    uint32 eset_full_status_mode;

    rv = soc_bm9600_eset_get(unit, ds_id, &low_eset_value, &hi_eset_value, &mc_full_eval_min, &eset_full_status_mode);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* It is the BCM layer resonsibility to convert node to modId */
    for (node = 0, num_members = 0; ((node < SB_FAB_DEVICE_BM9600_MAX_NODES) & (num_members < max_count)); node++) {
        if (node <= SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD) {
            if (COMPILER_64_BITTEST(low_eset_value, node)) {
                *(dist_modids + num_members++) = node;
            }
        }
        else {
            if (hi_eset_value & (1 << (node - SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD))) {
                *(dist_modids + num_members++) = node;
            }
        }
    }
    *count = num_members;

    return rv;
}

int
bcm_bm9600_fabric_distribution_control_set(int unit,
                       bcm_fabric_distribution_t ds_id,
                       bcm_fabric_distribution_control_t type,
                       int value)
{
    int rv = BCM_E_NONE;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    uint32 mc_full_eval_min;
    bcm_fabric_distribution_t nef_ds_id;
    uint32 eset_full_status_mode;
    int32 ds_id_region, cos_ds_id, fcd;

    rv = soc_bm9600_eset_get(unit, ds_id, &low_eset_value, &hi_eset_value, &mc_full_eval_min, &eset_full_status_mode);
    if (rv != BCM_E_NONE) {
        return(rv);
    }
    mc_full_eval_min = value;

    if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
        rv = soc_bm9600_eset_set(unit, ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 0);
        if (rv != BCM_E_NONE) {
            return(rv);
        }

        if (soc_bm9600_features(unit, soc_feature_egr_multicast_independent_fc)) {
            nef_ds_id = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, ds_id);
            rv = soc_bm9600_eset_set(unit, nef_ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 1);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
        }
    }
    else {
        for (ds_id_region = 0; ds_id_region < SOC_SBX_CFG(unit)->num_res_per_eset_spec;
                                                                            ds_id_region++) {
            rv = bcm_sbx_cosq_eset_to_cos_eset_fcd(unit, ds_id, ds_id_region, &cos_ds_id, &fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            rv = soc_bm9600_eset_set(unit, cos_ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
        }
    }

    return rv;
}

int
bcm_bm9600_fabric_distribution_control_get(int unit,
                       bcm_fabric_distribution_t ds_id,
                       bcm_fabric_distribution_control_t type,
                       int *value)
{
    int rv = BCM_E_NONE;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    uint32 mc_full_eval_min;
    uint32 eset_full_status_mode;

    rv = soc_bm9600_eset_get(unit, ds_id, &low_eset_value, &hi_eset_value, &mc_full_eval_min, &eset_full_status_mode);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    return rv;
}

int
bcm_bm9600_fabric_control_set(int unit, 
                              bcm_fabric_control_t type,
                              int arg)
{
    int bEnableAutoFailover;
    int bEnableAutoLinkDisable;
    int bEnableUseGlobalLink;
    uint32 uData;
    int rv = BCM_E_NONE;
    int node, modid;

    switch (type) {
        case bcmFabricArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, LOCAL_BM_ID, uData, arg);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, DEFAULT_BM_ID, uData, arg);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);            
            break;
        case bcmFabricArbiterConfig:
            SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG0, FORCE_NULL_GRANT, (arg == 0));
            break;
        case bcmFabricMaximumFailedLinks:
            /* update the max failed link configuration */
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData, arg);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);

            SOC_SBX_CFG(unit)->uMaxFailedLinks = arg;

        /* Unlike Petronius, I believe we don't need to change the degraded timeslot size */

            break;
        case bcmFabricManager:
        switch (arg) {
        case bcmFabricModeArbiterCrossbar:
            if (soc_feature(unit, soc_feature_arbiter_capable)) {
            /* Handle arbiter crossbar mode here - reinitialize BM9600 without affecting Crossbar links */
            SOC_FEATURE_SET(unit, soc_feature_arbiter_capable);
            SOC_FEATURE_SET(unit, soc_feature_arbiter);
            SOC_SBX_CFG_BM9600(unit)->uDeviceMode = SOC_SBX_BME_ARBITER_XBAR_MODE;
            SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = TRUE;
            LOG_CLI((BSL_META_U(unit,
                                "arbiter capable crossbar, setting device mode to %d\n"), SOC_SBX_CFG_BM9600(unit)->uDeviceMode));

            for (node = 0; node < SBX_MAXIMUM_NODES; node++) {
                
                modid = soc_property_port_get(unit, node, spn_SCI_PORT_MODID,
                              (BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->p2l_node[node]));
                
                if (modid != (BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->p2l_node[node])) {

                SOC_SBX_CFG(unit)->l2p_node[BCM_STK_MOD_TO_NODE(modid)] = node;
                SOC_SBX_CFG(unit)->p2l_node[node] = BCM_STK_MOD_TO_NODE(modid);
                
                if (SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)]
                    == BCM_STK_MOD_TO_NODE(modid)) {
                    SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)] = SBX_MAXIMUM_NODES - 1;
                }
                }
            }
            rv = soc_bm9600_init(unit, SOC_SBX_CFG(unit));

            } else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unit(%d) BM9600 is not capable of converting to an arbiter/crossbar device\n"),
                       unit));
            return BCM_E_UNAVAIL;
            }
            break;
        case  bcmFabricModeCrossbar:
            /* Handle crossbar only mode here */
            SOC_SBX_CFG_BM9600(unit)->uDeviceMode = SOC_SBX_BME_ARBITER_XBAR_MODE;
            SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = TRUE;
            rv = soc_bm9600_init(unit, SOC_SBX_CFG(unit));
            break;
            /* currently unsupported mode */
        case bcmFabricModeArbiterCapableCrossbar:
        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unit(%d) bcm_fabric_control_set type bcmFabricManager, unsupported mode\n"),
                       unit));
            rv = BCM_E_PARAM;
            break;
        }
        break;
        case bcmFabricRedundancyMode:
        /* configure redundancy mode */
            bEnableAutoFailover = 0;
            bEnableAutoLinkDisable = 0;
            bEnableUseGlobalLink = 0;
            switch (arg) {
                case bcmFabricRed1Plus1Both:
                    bEnableAutoFailover = 1;
                    break;
                case bcmFabricRed1Plus1LS:
                    bEnableUseGlobalLink = 1; /* intentional fall through */
                case bcmFabricRed1Plus1ELS:
                    bEnableAutoFailover = 1;
                    bEnableAutoLinkDisable = 1;
                    break;
                case bcmFabricRedLS:
                    bEnableUseGlobalLink = 1; /* intentional fall through */
                case bcmFabricRedELS:
                    bEnableAutoLinkDisable = 1;
                    break;
                case bcmFabricRedManual:
                    break;
                default:
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "bcmFabricRedundancyMode %d not supported by bm9600\n"), arg));
                    return BCM_E_PARAM;
            }
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);

            /* Update h/w configuration for node failure detection */
            if ( (arg == bcmFabricRed1Plus1ELS) || (arg == bcmFabricRedELS) ) {
                uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData, 24);
            }
            else {
                uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData, SOC_SBX_CFG(unit)->uMaxFailedLinks);
            }

            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AUTO_FAILOVER_ENABLE, uData, bEnableAutoFailover);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, USE_GLOBAL_LINK_ENABLE, uData, bEnableUseGlobalLink);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AUTO_LINK_DIS_ENABLE, uData, bEnableAutoLinkDisable);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);

        /* arm interrupts */
        uData = 0xFFFFFFFF;
        if (bEnableAutoFailover) {
        uData &= ~(SAND_HAL_PL_FO_EVENT_MASK_AUTO_FAILOVER_DISINT_MASK);
        }
        if (bEnableAutoLinkDisable) {
        uData &= ~(SAND_HAL_PL_FO_EVENT_MASK_AUTO_LINK_DIS_DISINT_MASK);
        uData &= ~(SAND_HAL_PL_FO_EVENT_MASK_AUTO_QE_DIS_DISINT_MASK);
        }
        SAND_HAL_WRITE(unit, PL, FO_EVENT_MASK, uData);

        /* clear events */
        uData = (SAND_HAL_PL_FO_EVENT_AUTO_FAILOVER_MASK |
             SAND_HAL_PL_FO_EVENT_AUTO_QE_DIS_MASK |
             SAND_HAL_PL_FO_EVENT_AUTO_LINK_DIS_MASK);
        SAND_HAL_WRITE(unit, PL, FO_EVENT, uData);

            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            return(BCM_E_UNAVAIL);
            break;

        case bcmFabricQueueMin:
            if (arg) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d: Invalid value specified (%d) for "
                                       "bcmFabricQueueMin. Only 0 is supported. \n"), unit, arg));
                rv = BCM_E_PARAM;
                break;
            } 
            /* nothing to do. Already set to 0. */
            break;
        case bcmFabricQueueMax:
            if ((arg < 0) || (arg > BM9600_BW_MAX_VOQ_NUM)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%d: Invalid Queue Max (%d) specified. Valid range: "
                                       "0-%d \n"), unit, arg, BM9600_BW_MAX_VOQ_NUM));
                rv = BCM_E_PARAM;
                break;
            }
            rv = soc_bm9600_epoch_in_timeslot_config_get(unit, arg, &uData);
            if (rv == SOC_E_NONE) {
                SOC_SBX_CFG(unit)->num_queues = arg;
                SOC_SBX_CFG(unit)->epoch_length_in_timeslots = uData;

                /* Adjust the sw state to reflect this new settings */
                rv = _bcm_sbx_cosq_queue_regions_set(unit);
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "%d: Could not set queue_regions to reflect new "
                                           "Max VOQs (%d) \n"), unit, arg));
                    break;
                }

                /* set the HW config registers */
                uData = SAND_HAL_READ(unit, PL, BW_EPOCH_CONFIG);
                uData = SAND_HAL_MOD_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
                          uData, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
                SAND_HAL_WRITE(unit, PL, BW_EPOCH_CONFIG, uData);

                uData = SAND_HAL_READ(unit, PL, BW_GROUP_CONFIG);
                uData = SAND_HAL_MOD_FIELD(PL, BW_GROUP_CONFIG, NUM_GROUPS, 
                          uData, (SOC_SBX_CFG(unit)->num_queues -1));
                SAND_HAL_WRITE(unit, PL, BW_GROUP_CONFIG, uData);
            } else {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%d: Could not calculate Epoch length for specified "
                                       "bcmFabricQueueMax value (%d) \n"), unit, arg));
                break;
            }
            break;
        case bcmFabricEgressQueueMin: /* intentional fall thru */
        case bcmFabricEgressQueueMax:
    case bcmFabricDemandCalculationEnable:
            rv = BCM_E_UNAVAIL;
            break;
        case bcmFabricOperatingIntervalEnable:
        /* Enable or disable on standby arbiter */
        uData = SAND_HAL_READ(unit, PL, BW_EPOCH_CONFIG);
        
        if (arg == TRUE) {
        uData = SAND_HAL_MOD_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
                       uData, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
        } else {
        uData = SAND_HAL_MOD_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
                       uData, 0);

        }
        SAND_HAL_WRITE(unit, PL, BW_EPOCH_CONFIG, uData);
        break;

        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d: Unsupported fabric control type(%d) "
                                   "specified \n"), unit, type));
            rv = BCM_E_PARAM;
    }

    return rv;
}

int
bcm_bm9600_fabric_control_get(int unit, 
                              bcm_fabric_control_t type,
                              int *arg)
{
    int     rv = BCM_E_NONE;
    uint32  uData;

    switch (type) {
        case bcmFabricArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_CONFIG0, LOCAL_BM_ID, uData);
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_CONFIG0, DEFAULT_BM_ID, uData);
            break;
        case bcmFabricArbiterConfig:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = (SAND_HAL_GET_FIELD(PL, FO_CONFIG0, FORCE_NULL_GRANT, uData) == 0);
            break;
        case bcmFabricMaximumFailedLinks:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData);
            break;
        case bcmFabricActiveId:
            uData = SAND_HAL_READ(unit, PL, FO_STATUS0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_STATUS0, ACTIVE_BM, uData);     
            break;
        case bcmFabricRedundancyMode:
            /* return cached value */
            *arg = SOC_SBX_CFG(unit)->uRedMode;
            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            rv = BCM_E_UNAVAIL;
            break;

        case bcmFabricQueueMin:
            *arg = 0; /* always starts at 0 */
            break;
        case bcmFabricQueueMax:
            *arg = SOC_SBX_CFG(unit)->num_queues;
            break;
        case bcmFabricEgressQueueMin: /* intentional fall thru */
        case bcmFabricEgressQueueMax:
            rv = BCM_E_UNAVAIL;
            break;

        default:
            rv = BCM_E_PARAM;
    }

    return rv;
}


/* CHANGES, START */

#if 0
int
bcm_bm9600_lcm_crossbar_connection_set(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_porta,
                                          int dst_portb)
{
    int rv;
    uint32 remap_entry;

    /* The bcm_port_t is a physical port, which is split into two logical ports. We map both
     * logical ports to the equivalent logical ports on the other side of the crossbar.
     */
    remap_entry = ((dst_portb & 0xff) << 8) | (dst_porta & 0xff);
    rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, 71, src_xbport * 2, remap_entry);
    if (rv != SOC_E_NONE) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d\n"), src_xbport));
      return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}

int
bcm_bm9600_lcm_crossbar_connection_get(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    int rv;

    if (!BCM_STK_MOD_IS_NODE(dst_modid)) {
        return BCM_E_PARAM;
    }

    /* The two logical ports should be mapped near-identically, so we just read the A grant */
    rv = soc_bm9600_XbXcfgRemapSelectRead(unit, BCM_STK_MOD_TO_NODE(dst_modid),
                                          src_xbport * 2, (uint32*)dst_xbport);
    *dst_xbport /= 2;
    if (rv != SOC_E_NONE) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n"), src_xbport));
      return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}
#endif /* 0 */

/* CHANGES, END */


