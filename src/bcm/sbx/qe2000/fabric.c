/*
 * $Id: fabric.c,v 1.35 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Fabric Control API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>


int
bcm_qe2000_fabric_crossbar_connection_set(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t dst_xbport)
{
    int uPlane, link;
    int rv, remapped;

    if (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE) {
      /* On a BM9600, the destination port is remapped in the crossbar 
       */
        return BCM_E_UNAVAIL;      
    }

    if (SOC_SBX_CONTROL(unit)->node_id != BCM_STK_MOD_TO_NODE(src_modid)) {
        /* do nothing if the modid doesn't match */
        return BCM_E_NONE;
    } else {
	
	/* Sequencing requirement:
	 *   bcm_fabric_crossbar_mapping_set has to be issued before this API
	 */

	remapped = FALSE;
	for (uPlane = 0; uPlane < HW_QE2000_NUM_SCI_LINKS; uPlane++) {
	    rv = bcm_fabric_crossbar_mapping_get(unit, src_modid, uPlane, xbar, &link);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "bcm_qe2000_fabric_crossbar_connection_set failed on plane %d xbar %d\n"),
                           uPlane, xbar));
		return rv;
	    }

	    if (link != -1) {
		rv = soc_qe2000_sfi_rd_wr_xcfg( unit, 0, link, BCM_STK_MOD_TO_NODE(dst_modid), &dst_xbport );
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "soc_qe2000_sfi_rd_wr_xcfg failed on plane %d xbar %d\n"),
                               uPlane, xbar));
		    return rv;
		}
		remapped = TRUE;
	    }
	}

	/* if neither plane has the remap setup, then return error */
	if (remapped == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "logical xbar %d has no physical link mapped on either plane, setup remap first\n"),
                       xbar));
	    return BCM_E_PARAM;
	} else {
	    return BCM_E_NONE;
	}
    }
}

int
bcm_qe2000_fabric_crossbar_connection_get(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    int uPlane, link;
    int rv, remapped;

    if (SOC_SBX_CONTROL(unit)->node_id != BCM_STK_MOD_TO_NODE(src_modid)) {
        /* return error if modid doesn't match, helps bcmx */
        return BCM_E_PARAM;
    } else {
	/* Sequencing requirement:
	 *   bcm_fabric_crossbar_mapping_set has to be issued before this API
	 */

	remapped = FALSE;
	for (uPlane = 0; uPlane < HW_QE2000_NUM_SCI_LINKS; uPlane++) {
	    rv = bcm_fabric_crossbar_mapping_get(unit, src_modid, uPlane, xbar, &link);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "bcm_qe2000_fabric_crossbar_connection_set failed on plane %d xbar %d\n"),
                           uPlane, xbar));
		return rv;
	    }

	    if (link != -1) {
		rv = soc_qe2000_sfi_rd_wr_xcfg( unit, 1, link, BCM_STK_MOD_TO_NODE(dst_modid), dst_xbport );
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "soc_qe2000_sfi_rd_wr_xcfg failed on plane %d xbar %d\n"),
                               uPlane, xbar));
		    return rv;
		}
		remapped = TRUE;
	    }
	}

	/* if neither plane has the remap setup, then return error */
	if (remapped == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "logical xbar %d has no physical link mapped on either plane, setup remap first\n"),
                       xbar));
	    return BCM_E_PARAM;
	} else {
	    return BCM_E_NONE;
	}
    }
}

int
bcm_qe2000_fabric_crossbar_mapping_set(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t port)
{
    uint32 link, shift, uData;
    uint32 mask = 0x1F;

    if (SOC_SBX_CONTROL(unit)->node_id != BCM_STK_MOD_TO_NODE(modid)) {
        /* do nothing if the modid doesn't match */
        return BCM_E_NONE;
    }
    if (xbar < 0 || xbar >= HW_QE2000_NUM_SFI_LINKS) {
	if (xbar == -1) {
	    /* Disable the remaps if port is -1 */
	    xbar = 0x1F;
	} else {
	    return BCM_E_PARAM;
	}
    }

    /* consistency check. Make sure a logical crossbar marked for custom modules is not used */
    if (SOC_SBX_CFG(unit)->module_custom1_links & (1 << xbar)) {
        return(BCM_E_PARAM);
    }

    port = SOC_PORT_BLOCK_INDEX(unit, port);
    if (port < 0 || port >= HW_QE2000_NUM_SFI_LINKS) {
	return BCM_E_PARAM;
    }

    

    /* disable the xbar remap in the link_enable_remap for the port */
    shift = (port % 6) * 5;
    if ( switch_fabric_arbiter_id == 0 ) {
	if ( port < 6 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_ENABLE_REMAP0);
	    uData |= (0x1F << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_ENABLE_REMAP0, uData);
	} else if ( port < 12 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_ENABLE_REMAP1);
	    uData |= (0x1F << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_ENABLE_REMAP1, uData);
	} else {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_ENABLE_REMAP2);
	    uData |= (0x1F << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_ENABLE_REMAP2, uData);
	}
    } else {
	if ( port < 6 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_ENABLE_REMAP0);
	    uData |= (0x1F << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_ENABLE_REMAP0, uData);
	} else if ( port < 12 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_ENABLE_REMAP1);
	    uData |= (0x1F << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_ENABLE_REMAP1, uData);
	} else {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_ENABLE_REMAP2);
	    uData |= (0x1F << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_ENABLE_REMAP2, uData);
	}
    }


    /* loop through all links, if the port remap in the link_status_remap matches
     * to the port specified, init the setting to 0x1F(disable)
     */
    for (link = 0; link < HW_QE2000_NUM_SFI_LINKS; link++) {
	shift = (link % 6) * 5;
	if ( switch_fabric_arbiter_id == 0 ) {
	    if ( link < 6 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP0);
		if (((uData & (mask << shift))>>shift) == port) {
		    uData |= (0x1F << shift);
		    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_STATUS_REMAP0, uData);
		}
	    } else if ( link < 12 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP1);
		if (((uData & (mask << shift))>>shift) == port) {
		    uData |= (0x1F << shift);
		    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_STATUS_REMAP1, uData);
		}
	    } else {
		uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP2);
		if (((uData & (mask << shift))>>shift) == port) {
		    uData |= (0x1F << shift);
		    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_STATUS_REMAP2, uData);
		}
	    }

	} else {
	    if ( link < 6 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP0);
		if (((uData & (mask << shift))>>shift) == port) {
		    uData |= (0x1F << shift);
		    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_STATUS_REMAP0, uData);
		}
	    } else if ( link < 12 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP1);
		if (((uData & (mask << shift))>>shift) == port) {
		    uData |= (0x1F << shift);
		    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_STATUS_REMAP1, uData);
		}
	    } else {
		uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP2);
		if (((uData & (mask << shift))>>shift) == port) {
		    uData |= (0x1F << shift);
		    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_STATUS_REMAP2, uData);
		}
	    }
	}
    }

    if ( switch_fabric_arbiter_id == 0 ) {
	/* Enable remap physical crossbar to logical crossbar */
	shift = (port % 6) * 5;
	if ( port < 6 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_ENABLE_REMAP0);
	    uData &= ~(mask << shift);
	    uData |= ((xbar & mask) << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_ENABLE_REMAP0, uData);
	} else if ( port < 12 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_ENABLE_REMAP1);
	    uData &= ~(mask << shift);
	    uData |= ((xbar & mask) << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_ENABLE_REMAP1, uData);
	} else {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_ENABLE_REMAP2);
	    uData &= ~(mask << shift);
	    uData |= ((xbar & mask) << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_ENABLE_REMAP2, uData);
	}

	if (xbar != 0x1F) {
	    /* Status remap logical crossbar to physical crossbar */
	    shift = (xbar % 6) * 5;
	    if ( xbar < 6 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP0);
		uData &= ~(mask << shift);
		uData |= ((port & mask) << shift);
		SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_STATUS_REMAP0, uData);
	    } else if ( xbar < 12 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP1);
		uData &= ~(mask << shift);
		uData |= ((port & mask) << shift);
		SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_STATUS_REMAP1, uData);
	    } else {
		uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP2);
		uData &= ~(mask << shift);
		uData |= ((port & mask) << shift);
		SAND_HAL_WRITE(unit, KA, SC_SCI0_LINK_STATUS_REMAP2, uData);
	    }
	}

    } else {
	/* Enable remap physical crossbar to logical crossbar */
	shift = (port % 6) * 5;
	if ( port < 6 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_ENABLE_REMAP0);
	    uData &= ~(mask << shift);
	    uData |= ((xbar & mask) << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_ENABLE_REMAP0, uData);
	} else if ( port < 12 ) {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_ENABLE_REMAP1);
	    uData &= ~(mask << shift);
	    uData |= ((xbar & mask) << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_ENABLE_REMAP1, uData);
	} else {
	    uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_ENABLE_REMAP2);
	    uData &= ~(mask << shift);
	    uData |= ((xbar & mask) << shift);
	    SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_ENABLE_REMAP2, uData);
	}

	if (xbar != 0x1F) {
	    /* Status remap logical crossbar to physical crossbar */
	    shift = (xbar % 6) * 5;
	    if ( xbar < 6 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP0);
		uData &= ~(mask << shift);
		uData |= ((port & mask) << shift);
		SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_STATUS_REMAP0, uData);
	    } else if ( xbar < 12 ) {
		uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP1);
		uData &= ~(mask << shift);
		uData |= ((port & mask) << shift);
		SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_STATUS_REMAP1, uData);
	    } else {
		uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP2);
		uData &= ~(mask << shift);
		uData |= ((port & mask) << shift);
		SAND_HAL_WRITE(unit, KA, SC_SCI1_LINK_STATUS_REMAP2, uData);
	    }
	}
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_fabric_crossbar_mapping_get(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t *port)
{
    uint32 shift, uData;
    uint32 mask = 0x1F;

    if (SOC_SBX_CONTROL(unit)->node_id != BCM_STK_MOD_TO_NODE(modid)) {
        /* return error if modid doens't match, helps bcmx */
        return BCM_E_PARAM;
    }
    if (xbar < 0 || xbar >= HW_QE2000_NUM_SFI_LINKS) {
        return BCM_E_PARAM;
    }

    shift = (xbar % 6) * 5;

    if ( switch_fabric_arbiter_id == 0 ) {
        if ( xbar < 6 ) {
            uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP0);
        } else if ( xbar < 12 ) {
            uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP1);
        } else {
            uData = SAND_HAL_READ(unit, KA, SC_SCI0_LINK_STATUS_REMAP2);
        }
    } else {
        if ( xbar < 6 ) {
            uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP0);
        } else if ( xbar < 12 ) {
            uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP1);
        } else {
            uData = SAND_HAL_READ(unit, KA, SC_SCI1_LINK_STATUS_REMAP2);
        }
    }

    /* Enable remap logical crossbar to physical crossbar */
    *port = ((uData & (mask << shift)) >> shift);

    if (*port == 0x1F) {
	/* return -1 if the link remap is disabled */
	*port = -1;
    } else {
	*port += SOC_PORT_MIN(unit, sfi);
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_fabric_crossbar_enable_set(int unit,
                                      uint64 xbars)
{
    int rv = BCM_E_NONE;
    uint64 uuNodeBytesPerEpoch = COMPILER_64_INIT(0,0);
    uint64 uuEpochLengthInNs;
    uint64 uuLengthSumAdjustInBytes = COMPILER_64_INIT(0,0);
    uint32 nodeBytesPerEpoch = 0;
    uint32 lengthSumAdjustInBytes = 0;
    uint32 i, uData;
    int32  nTsSizeNormNs;
    int32  nSfiCount;
    int32  nHalfBus;
    bcm_sbx_fabric_state_t *p_fabric_state;
    int32 nRateConversion;
    int32 bs_lines, cong_bs_lines;

    nHalfBus = SOC_SBX_CFG(unit)->bHalfBus;

    if (soc_feature(unit, soc_feature_standalone)) {
	return BCM_E_NONE;
    }

    /* set the link enables on the BME */
    nSfiCount = 0;
    for (i=0; i<64; i++) {
        /* Count number of enabled links */
      nSfiCount += COMPILER_64_BITTEST(xbars,i);
    }

    nTsSizeNormNs = soc_sbx_fabric_get_timeslot_size(unit, nSfiCount, nHalfBus, soc_feature(unit, soc_feature_hybrid) );

    p_fabric_state = (bcm_sbx_fabric_state_t*)(SOC_SBX_STATE(unit)->fabric_state);
    p_fabric_state->timeslot_size = nTsSizeNormNs;

#define BCM_QE2000_FABRIC_NANOSEC_PER_SEC  1000000
#define BCM_QE2000_FABRIC_MICROSEC_PER_SEC 1000
#define BCM_QE2000_FABRIC_BITS_PER_BYTE 8

    COMPILER_64_SET(uuEpochLengthInNs, 0, p_fabric_state->timeslot_size);
    COMPILER_64_UMUL_32(uuEpochLengthInNs,  SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
    COMPILER_64_UMUL_32(uuEpochLengthInNs, 20000000); /* 20 Gbps */
    
    if(soc_sbx_div64((uuEpochLengthInNs), (BCM_QE2000_FABRIC_NANOSEC_PER_SEC * BCM_QE2000_FABRIC_BITS_PER_BYTE), 
		     &nodeBytesPerEpoch) == -1) {
      return BCM_E_INTERNAL;
    }

    COMPILER_64_SET(uuNodeBytesPerEpoch, 0, nodeBytesPerEpoch);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "timeslot_size(%d) epoch_length_in_timeslots(%d) epoch_length_in_ns(0x%x%08x)\n"),
              p_fabric_state->timeslot_size, SOC_SBX_CFG(unit)->epoch_length_in_timeslots, COMPILER_64_HI(uuEpochLengthInNs), COMPILER_64_LO(uuEpochLengthInNs)));

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "uuNodeBytesPerEpoch(0x%x%08x)\n"),
              COMPILER_64_HI(uuNodeBytesPerEpoch),COMPILER_64_LO(uuNodeBytesPerEpoch)));

    /* In D-mode, length_sum_adjust represents the amount of data (in bytes)     */
    /* that can arrive at a queue between the time that the first "pass" of the  */
    /* demand calcuation is run and the time that the second "pass" of the       */
    /* demand calculation is run.  This amount of time is roughly 3*number of    */
    /* queues*cycle time.  So, normally, it is 3*16K*4 ns, or 197 usec.  So,     */
    /* you'd need to figure out how many bytes of data can arrive for all queues */
    /* in that amount of time. That is the max port rate in bytes * 197 usec     */
    /* GNATS 23943 divide by 8 because field is not in bits, but is in bytes     */

    
    COMPILER_64_SET(uuLengthSumAdjustInBytes, 0, 20000000 * 3); /* 20 Gbps * 3 */
    COMPILER_64_UMUL_32(uuLengthSumAdjustInBytes, SB_FAB_DEVICE_QE2000_NUM_QUEUES);
    if (soc_sbx_div64(uuLengthSumAdjustInBytes,
		      (8 * BCM_QE2000_FABRIC_MICROSEC_PER_SEC), &lengthSumAdjustInBytes) == -1) {
	return BCM_E_INTERNAL;
    }
    
    SAND_HAL_WRITE(unit, KA, QM_DC_CONFIG0,
		   SAND_HAL_SET_FIELD(KA, QM_DC_CONFIG0, MAX_ARRIVAL_BYTES, nodeBytesPerEpoch));

    SAND_HAL_WRITE(unit, KA, QM_DC_CONFIG1,
		   SAND_HAL_SET_FIELD(KA, QM_DC_CONFIG1, LENGTH_SUM_ADJUST, lengthSumAdjustInBytes));

    /* Determine the rate conversion value and update */
    nRateConversion = bcm_sbx_fabric_get_rate_conversion(unit, nSfiCount);
    
    if (nRateConversion == -1) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "nRateConversion failed\n")));
      return BCM_E_INTERNAL;
    }

    SAND_HAL_WRITE(unit, KA, QS_RATE_CONVERSION, nRateConversion);

    if ( SOC_SBX_CFG(unit)->demand_scale ) {
	uData = SAND_HAL_READ(unit, KA, QM_CONFIG1);
	uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG1, RATE_SHIFT, uData, SOC_SBX_CFG(unit)->demand_scale);
	SAND_HAL_WRITE(unit, KA, QM_CONFIG1, uData);	
    }

   
#if 0
   soc_qe2000_rate_delta_config(unit, p_fabric_state->timeslot_size,
                                      SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
#endif /* 0 */

    /* update burst size */
    if (SOC_SBX_CFG(unit)->module_custom1_in_system == TRUE) {
        rv = soc_qe2000_burst_size_lines_get(unit, &bs_lines, &cong_bs_lines, xbars);
        if (rv != BCM_E_NONE) {
            return(rv);
        }
        
        /* PM configuration */
        uData = SAND_HAL_READ(unit, KA, PM_CONFIG1);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES1_QE2K, uData, bs_lines);
        uData = SAND_HAL_MOD_FIELD(KA, PM_CONFIG1, NUM_FAB_LINES2_QE2K, uData, cong_bs_lines);
        SAND_HAL_WRITE(unit, KA, PM_CONFIG1, uData);

        /* QMGR configuration */
        uData = SAND_HAL_READ(unit, KA, QM_CONFIG2);
        uData = SAND_HAL_MOD_FIELD(KA, QM_CONFIG2, LINES_PER_TS_QE2K, uData, cong_bs_lines);
        SAND_HAL_WRITE(unit, KA, QM_CONFIG2, uData);
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_fabric_crossbar_enable_get(int unit,
                                      uint64 *xbars)
{
    return BCM_E_UNAVAIL;
}

int
bcm_qe2000_fabric_crossbar_status_get(int unit,
                                      uint64 *xbars)
{
    return BCM_E_UNAVAIL;
}

int
bcm_qe2000_fabric_distribution_create(int unit,
                                      bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_qe2000_fabric_distribution_destroy(int unit,
                                       bcm_fabric_distribution_t  ds_id)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_qe2000_fabric_distribution_set(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int modid_count,
                                   int *dist_modids)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_qe2000_fabric_distribution_get(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int max_count,
                                   int *dist_modids,
                                   int *count)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_qe2000_fabric_packet_adjust_set(int unit,
                                    int pkt_adjust_selector,
                                    int pkt_adjust_len)
{
    int rv = BCM_E_NONE;
    int reg_index, reg_field;
    uint32 uData, field_value, mask_value, flags;

    /* figure out which way to set, 'neither' means 'all' */
    flags = pkt_adjust_selector & ~(BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK);
    if (!(flags & (BCM_FABRIC_PACKET_ADJUST_EGRESS |
                   BCM_FABRIC_PACKET_ADJUST_INGRESS))) {
        flags |= (BCM_FABRIC_PACKET_ADJUST_EGRESS |
                  BCM_FABRIC_PACKET_ADJUST_INGRESS);
    }
    /* get the correct adjust index to set */
    pkt_adjust_selector &= BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK;

    if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) {
        /* adjusting the global settings */
        if (pkt_adjust_selector) {
            /* there's only one global per direction, nonzero is wrong */
            return BCM_E_PARAM;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /* ingress - unsigned 6b adjustment with sign bit */
            if ((-63 <= pkt_adjust_len) && (63 >= pkt_adjust_len)) {
                if (0 <= pkt_adjust_len) {
                    field_value = pkt_adjust_len;
                } else {
                    field_value = -pkt_adjust_len;
                }
                uData = SAND_HAL_READ(unit, KA, QM_CONFIG2);
                uData = SAND_HAL_MOD_FIELD(KA,
                                           QM_CONFIG2,
                                           GLOBAL_HDR_ADJUST_SIGN,
                                           uData,
                                           (0 > pkt_adjust_len));
                uData = SAND_HAL_MOD_FIELD(KA,
                                           QM_CONFIG2,
                                           GLOBAL_HDR_ADJUST,
                                           uData,
                                           field_value);
                SAND_HAL_WRITE(unit, KA, QM_CONFIG2, uData);
            } else { /* if ((-63 <= pkt_adjust_len) && (63 >= pkt_adjust_len)) */
                rv = BCM_E_PARAM;
            } /* if ((-63 <= pkt_adjust_len) && (63 >= pkt_adjust_len)) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /* egress - signed 8b adjustment */
            if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) {
                field_value = pkt_adjust_len & 0xFF;
                uData = SAND_HAL_READ(unit, KA, EP_BF_CONFIG);
                uData = SAND_HAL_MOD_FIELD(KA,
                                           EP_BF_CONFIG,
                                           GLOBAL_LEN_ADJ,
                                           uData,
                                           field_value);
                SAND_HAL_WRITE(unit, KA, EP_BF_CONFIG, uData);
            } else { /* if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) */
                rv = BCM_E_PARAM;
            } /* if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } else { /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
        /* adjusting an individual template */
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /*
             *  Ingress side -- numbers are unsigned 7b with a sign bit.   The
             *  adjust values are mapped four to a register, with register 0 ==
             *  {15,13,13,12} through 3 == {3,2,1,0}.  '-0' is invalid.
             */
            if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) {
                reg_index = (pkt_adjust_selector / 4);
                reg_field = (pkt_adjust_selector % 4);
                if (pkt_adjust_len >= 0) {
                    field_value = pkt_adjust_len & 0x7F;
                } else {
                    field_value = ((-pkt_adjust_len) & 0x7F) | 0x80;
                }
                field_value = field_value & 0xFF;
                field_value = (field_value << (reg_field * 8));
                mask_value = (0xFF << (reg_field * 8));

                switch (reg_index) {
                case 0:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST3);
                    uData = (uData & ~mask_value) | field_value;
                    SAND_HAL_WRITE(unit, KA, QM_PKT_HDR_ADJUST3, uData);
                    break;
                case 1:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST2);
                    uData = (uData & ~mask_value) | field_value;
                    SAND_HAL_WRITE(unit, KA, QM_PKT_HDR_ADJUST2, uData);
                    break;
                case 2:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST1);
                    uData = (uData & ~mask_value) | field_value;
                    SAND_HAL_WRITE(unit, KA, QM_PKT_HDR_ADJUST1, uData);
                    break;
                case 3:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST0);
                    uData = (uData & ~mask_value) | field_value;
                    SAND_HAL_WRITE(unit, KA, QM_PKT_HDR_ADJUST0, uData);
                    break;
                default:
                    rv = BCM_E_PARAM;
                } /* switch (reg_index) */
            } else { /* if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) */
                /* out of range */
                rv = BCM_E_PARAM;
            } /* if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /*
             *  Egress side -- numbers are signed 8b, one per register, all in
             *  the low byte (the upper three bytes are 'reserved').
             */
            if ((-127 <= pkt_adjust_len) && (127 >= pkt_adjust_len)) {
                field_value = pkt_adjust_len & 0xFF;
                switch (pkt_adjust_selector) {
                case 0:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ0) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ0, uData);
                    break;
                case 1:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ1) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ1, uData);
                    break;
                case 2:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ2) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ2, uData);
                    break;
                case 3:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ3) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ3, uData);
                    break;
                case 4:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ4) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ4, uData);
                    break;
                case 5:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ5) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ5, uData);
                    break;
                case 6:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ6) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ6, uData);
                    break;
                case 7:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ7) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ7, uData);
                    break;
                case 8:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ8) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ8, uData);
                    break;
                case 9:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ9) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ9, uData);
                    break;
                case 10:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ10) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ10, uData);
                    break;
                case 11:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ11) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ11, uData);
                    break;
                case 12:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ12) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ12, uData);
                    break;
                case 13:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ13) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ13, uData);
                    break;
                case 14:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ14) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ14, uData);
                    break;
                case 15:
                    uData = (SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ15) & (~(0xFF))) | field_value;
                    SAND_HAL_WRITE(unit, KA, EP_BF_LEN_ADJ15, uData);
                    break;
                default:
                    rv = BCM_E_PARAM;
                } /* switch (pkt_adjust_selector) */
            } else { /* if (adjust value is valid) */
                rv = BCM_E_PARAM;
            } /* if (adjust value is valid) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
    return(rv);
}

int
bcm_qe2000_fabric_packet_adjust_get(int unit,
                                    int pkt_adjust_selector,
                                    int *pkt_adjust_len)
{
    int rv = BCM_E_NONE;
    int reg_index, reg_field;
    uint32 uData, field_value, flags;

    uData = 0; 

    /*
     *  Figure out which way to read; 'neither' means 'ingress' unless ingress
     *  is not supported, in which case it means 'egress'.
     */
    flags = pkt_adjust_selector & ~(BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK);
    if (!(flags & (BCM_FABRIC_PACKET_ADJUST_EGRESS |
                   BCM_FABRIC_PACKET_ADJUST_INGRESS))) {
        flags |= BCM_FABRIC_PACKET_ADJUST_INGRESS;
    }
    if ((BCM_FABRIC_PACKET_ADJUST_EGRESS | BCM_FABRIC_PACKET_ADJUST_INGRESS) ==
        (flags & (BCM_FABRIC_PACKET_ADJUST_EGRESS |
                  BCM_FABRIC_PACKET_ADJUST_INGRESS))) {
        /* both directions is not valid for read */
        return BCM_E_PARAM;
    }
    /* get the correct adjust index to read */
    pkt_adjust_selector &= BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK;

    if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) {
        /* reading global adjustment */
        if (pkt_adjust_selector) {
            /* there's only one global per direction, nonzero is wrong */
            return BCM_E_PARAM;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /* ingress - unsigned 6b adjustment with sign bit */
            uData = SAND_HAL_READ(unit, KA, QM_CONFIG2);
            field_value = SAND_HAL_GET_FIELD(KA,
                                             QM_CONFIG2,
                                             GLOBAL_HDR_ADJUST,
                                             uData);
            if (SAND_HAL_GET_FIELD(KA,
                                   QM_CONFIG2,
                                   GLOBAL_HDR_ADJUST_SIGN,
                                   uData)) {
                field_value = -field_value;
            }
            *pkt_adjust_len = field_value;
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /* egress - signed 8b adjustment */
            uData = SAND_HAL_READ(unit, KA, EP_BF_CONFIG);
            field_value = SAND_HAL_GET_FIELD(KA,
                                             EP_BF_CONFIG,
                                             GLOBAL_LEN_ADJ,
                                             uData);
            if (field_value & 0x80) {
                /* negative; extend sign */
                field_value = (-1 & (~0xFF)) | field_value;
            }
            *pkt_adjust_len = field_value;
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } else { /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
        /* reading individual template */
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /*
             *  Ingress side -- numbers are unsigned 7b with a sign bit.   The
             *  adjust values are mapped four to a register, with register 0 ==
             *  {15,13,13,12} through 3 == {3,2,1,0}.  '-0' is invalid.
             */
            reg_index = (pkt_adjust_selector / 4);
            reg_field = (pkt_adjust_selector % 4);
            switch (reg_index) {
                case 0:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST3);
                    break;
                case 1:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST2);
                    break;
                case 2:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST1);
                    break;
                case 3:
                    uData = SAND_HAL_READ(unit, KA, QM_PKT_HDR_ADJUST0);
                    break;
                default:
                    rv = BCM_E_PARAM;
                    break;
            } /* switch (reg_index) */
            if (BCM_E_NONE == rv) {
                field_value = (uData >> (reg_field * 8)) & 0xFF;
                (*pkt_adjust_len) = (field_value & 0x80) ?
                                    -(field_value & 0x7F) :
                                    (field_value & 0x7F);
            }
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /*
             *  Egress side -- numbers are signed 8b, one per register, all in
             *  the low byte (the upper three bytes are 'reserved').
             */
            switch (pkt_adjust_selector) {
            case 0:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ0) & 0xFF;
                break;
            case 1:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ1) & 0xFF;
                break;
            case 2:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ2) & 0xFF;
                break;
            case 3:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ3) & 0xFF;
                break;
            case 4:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ4) & 0xFF;
                break;
            case 5:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ5) & 0xFF;
                break;
            case 6:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ6) & 0xFF;
                break;
            case 7:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ7) & 0xFF;
                break;
            case 8:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ8) & 0xFF;
                break;
            case 9:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ9) & 0xFF;
                break;
            case 10:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ10) & 0xFF;
                break;
            case 11:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ11) & 0xFF;
                break;
            case 12:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ12) & 0xFF;
                break;
            case 13:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ13) & 0xFF;
                break;
            case 14:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ14) & 0xFF;
                break;
            case 15:
                uData = SAND_HAL_READ(unit, KA, EP_BF_LEN_ADJ15) & 0xFF;
                break;
            default:
                rv = BCM_E_PARAM;
            } /* switch (pkt_adjust_selector) */
            if (BCM_E_NONE == rv) {
                if (uData & 0x80) {
                    /* it's signed and negative; extend the sign */
                    *pkt_adjust_len = (-1 & (~0xFF)) | uData;
                } else {
                    /* it's signed but nonnegative, leave it alone */
                    *pkt_adjust_len = uData;
                }
            } /* if (BCM_E_NONE == rv) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
    return(rv);
}

int
bcm_qe2000_fabric_control_set(int unit,
                              bcm_fabric_control_t type,
                              int arg)
{
    int bEnableAutoFailover;
    int bEnableOnePlusOne;
    uint32 uData;
    bcm_multicast_t group;
    int source_knockout;
    int rv;


    switch (type) {
        case bcmFabricArbiterId:
        case bcmFabricMaximumFailedLinks:
        case bcmFabricArbiterConfig:
            /* Not apply to QE2000 devices */
            return BCM_E_NONE;
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, KA, SC_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, DEFAULT_BM, uData, arg);
            SAND_HAL_WRITE(unit, KA, SC_CONFIG0, uData);
            break;
        case bcmFabricRedundancyMode:
            bEnableAutoFailover = 0;
            bEnableOnePlusOne = 0;
            switch (arg) {
                case bcmFabricRed1Plus1Both:
                    bEnableAutoFailover = 1;
                    bEnableOnePlusOne = 1;
                    break;
                case bcmFabricRed1Plus1LS:
                    bEnableAutoFailover = 1;
                    break;
                case bcmFabricRedLS:
                case bcmFabricRedManual:
                    break;
                case bcmFabricRed1Plus1ELS:
                case bcmFabricRedELS:
                default:
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "bcmFabricRedundancyMode %d not supported by bm3200\n"), arg));
                    return BCM_E_PARAM;
            }
            uData = SAND_HAL_READ(unit, KA, SC_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, ENABLE_AUTO_SWITCHOVER, uData, bEnableAutoFailover);
            uData = SAND_HAL_MOD_FIELD(KA, SC_CONFIG0, MODE, uData, bEnableOnePlusOne);
            SAND_HAL_WRITE(unit, KA, SC_CONFIG0, uData);

	    /* clear interrupts, re-arm redundancy */
	    uData = SAND_HAL_READ(unit, KA, SC_ERROR);
	    uData |= (SAND_HAL_KA_SC_ERROR_AUTO_SWITCH_EVENT_CTRL_MASK | 
		      SAND_HAL_KA_SC_ERROR_AUTO_SWITCH_EVENT_DATA_MASK);
	    SAND_HAL_WRITE(unit, KA, SC_ERROR, uData);

            break;
	case bcmFabricMode:
	    if (arg) {
            if (!(SOC_SBX_CFG(unit)->bTmeMode)) {
                /* already in FIC mode */
                return BCM_E_NONE;
            } else {
                /* switch QE to FIC mode */
                SOC_SBX_CFG(unit)->bTmeMode = 0;
            }
        } else {
            if (SOC_SBX_CFG(unit)->bTmeMode) {
                /* already in TME mode */
                return BCM_E_NONE;
            } else {
                /* switch QE to TME mode */
                SOC_SBX_CFG(unit)->bTmeMode = 1;
            }
        }

            soc_sbx_set_epoch_length(unit);
	    SOC_SBX_CFG(unit)->reset_ul = 1;

            
	    soc_qe2000_init(unit, SOC_SBX_CFG(unit));
        break;
        case bcmFabricMcGroupSourceKnockout:
            group = arg & BCM_FABRIC_MC_GROUP_MASK;
            source_knockout = (arg & BCM_FABRIC_MC_GROUP_SOURCE_KNOCKOUT_ENABLE) ? TRUE : FALSE;
            rv = bcm_qe2000_multicast_source_knockout_set(unit, group, source_knockout);
            return(rv);
            break;
	case bcmFabricDemandCalculationEnable:
	    uData = SAND_HAL_READ(unit, KA, QM_DEBUG_CONFIG);
	    if (arg == TRUE) {
		uData = SAND_HAL_MOD_FIELD(KA, QM_DEBUG_CONFIG, DMODE_DISABLE, uData, 0);
	    } else {
		uData = SAND_HAL_MOD_FIELD(KA, QM_DEBUG_CONFIG, DMODE_DISABLE, uData, 1);
	    }
	    SAND_HAL_WRITE(unit, KA, QM_DEBUG_CONFIG, uData);
	    break;

	case bcmFabricOperatingIntervalEnable:
        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            return(BCM_E_UNAVAIL);
            break;

        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_fabric_control_get(int unit,
                              bcm_fabric_control_t type,
                              int *arg)
{
    uint32 uData;
    int rv;
    bcm_multicast_t group;
    int source_knockout;


    switch (type) {
        case bcmFabricArbiterId:
        case bcmFabricMaximumFailedLinks:
        case bcmFabricArbiterConfig:
            /* Not apply to QE2000 devices */
            return BCM_E_UNAVAIL;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, KA, SC_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(KA, SC_CONFIG0, DEFAULT_BM, uData);
            break;
        case bcmFabricActiveId:
            uData = SAND_HAL_READ(unit, KA, SC_STATUS0);
            *arg = SAND_HAL_GET_FIELD(KA, SC_STATUS0, SELECTED_BM, uData);
            break;
        case bcmFabricRedundancyMode:
            /* return cached value */
            *arg = SOC_SBX_CFG(unit)->uRedMode;
            break;

        case bcmFabricMcGroupSourceKnockout:
            group = (*arg) & BCM_FABRIC_MC_GROUP_MASK;
            rv = bcm_qe2000_multicast_source_knockout_get(unit, group, &source_knockout);
            if (rv == BCM_E_NONE) {
                (*arg) &= BCM_FABRIC_MC_GROUP_MASK;
                (*arg) |= (source_knockout == TRUE) ? BCM_FABRIC_MC_GROUP_SOURCE_KNOCKOUT_ENABLE : 0;
            }
            return(rv);
            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            return(BCM_E_UNAVAIL);
            break;

        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

