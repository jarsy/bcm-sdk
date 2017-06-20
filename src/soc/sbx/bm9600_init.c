/* -*- mode:c; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
 * $Id: bm9600_init.c,v 1.103 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * bm9600_init.c : Configure BM9600 chip for Initialization
 *
 *-----------------------------------------------------------------------------*/
#include <shared/bsl.h>
#include <soc/types.h>
#include <soc/cm.h>
#include <soc/sbx/sbTypesGlue.h>

#ifdef BCM_BM9600_SUPPORT

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbFabCommon.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/fabric/sbZfFabBm9600NmEmtEntry.hx>

#include "bm9600_properties.h"
#include "bm9600_init.h"
#include "bm9600.h"
#include "bm9600_soc_init.h"

#ifdef  BM9600_BASE_ADDR
#undef  BM9600_BASE_ADDR
#endif
#define BM9600_BASE_ADDR pInitParams->m_nBaseAddr

#define printf  bsl_printf


static UINT bm9600IngressRankerTable[NM_INGRESS_RANKER_TABLE_NUM_ENTRIES] =
{
0x0, 0x1, 0x2, 0x3,
0x4, 0x5, 0x6, 0x7,
0x8, 0x9, 0xa, 0xb,
0xc, 0xd, 0xe, 0xf,
0x10, 0x11, 0x12, 0x13,
0x14, 0x15, 0x16, 0x17,
0x18, 0x19, 0x1a, 0x1b,
0x1c, 0x1d, 0x1e, 0x1f,
0x20, 0x21, 0x22, 0x23,
0x24, 0x25, 0x26, 0x27,
0x28, 0x29, 0x2a, 0x2b,
0x2c, 0x2d, 0x2e, 0x2f,
0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x3a, 0x3b,
0x3c, 0x3d, 0x3e, 0x3f,
0x40, 0x41, 0x42, 0x43,
0x44, 0x45, 0x46, 0x47,
0x48
};

static UINT bm9600EgressRankerTable[NM_EGRESS_RANKER_TABLE_NUM_ENTRIES] =
{
0x0, 0x1, 0x2, 0x3,
0x4, 0x5, 0x6, 0x7,
0x8, 0x9, 0xa, 0xb,
0xc, 0xd, 0xe, 0xf,
0x10, 0x11, 0x12, 0x13,
0x14, 0x15, 0x16, 0x17,
0x18, 0x19, 0x1a, 0x1b,
0x1c, 0x1d, 0x1e, 0x1f,
0x20, 0x21, 0x22, 0x23,
0x24, 0x25, 0x26, 0x27,
0x28, 0x29, 0x2a, 0x2b,
0x2c, 0x2d, 0x2e, 0x2f,
0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x3a, 0x3b,
0x3c, 0x3d, 0x3e, 0x3f,
0x40, 0x41, 0x42, 0x43,
0x44, 0x45, 0x46, 0x47,
0x48
};

static UINT bm9600RandTable[NM_RAND_NUM_TABLE_NUM_ENTRIES] =
{
0xe0f8, 0x1c89, 0x8eb0, 0xce19, 0x61b2, 0x35b0, 0x9eb9, 0x00f1,
0x1ffa, 0xdf29, 0xb3aa, 0x35f4, 0x07ed, 0xe463, 0x2461, 0xe5d4,
0xaa70, 0x2ca4, 0x53e0, 0x8755, 0x5731, 0xee3e, 0x2546, 0x610a,
0xf247, 0x6f55, 0xdca2, 0x25ee, 0xf08f, 0x3c87, 0x6526, 0xe313,
0x9479, 0x7a21, 0xd56a, 0x114d, 0xbce5, 0xd5aa, 0x4274, 0x6e2c,
0x413a, 0x5526, 0x2062, 0xe05c, 0xecd4, 0x67c1, 0x49b5, 0xaa24,
0xef53, 0x1c86, 0x06f3, 0xf52c, 0xebb9, 0x9319, 0x9901
};


#if 00

#define BM9600_LOG(format) do{printf(format); } while(0)
#define BM9600_LOG1(format, a1) do{printf(format, a1); } while(0)
#define BM9600_LOG2(format, a1, a2) do{printf(format, a1, a2); } while(0)
#define BM9600_LOG3(format, a1, a2, a3) do{printf(format, a1, a2, a3); } while(0)
#else
#define BM9600_LOG(format)
#define BM9600_LOG1(format, a1)
#define BM9600_LOG2(format, a1, a2);
#define BM9600_LOG3(format, a1, a2, a3);
#endif

#define BM9600_ERR(format) do{printf(format); } while(0)
#define BM9600_ERR1(format, a1) do{printf(format, a1); } while(0)
#define BM9600_ERR2(format, a1, a2) do{printf(format, a1, a2); } while(0)


#define ASSERT(x) \
  do { if (!(x)) printf("BM9600: assertion failed\n"); } while (0)



/*
 * local declarations
 */
uint32 hwBm9600GetTimeSlotSizeInClocks(const bm9600InitParams_t *pInitParams);
void hwBm9600TraceErrorRegs(char *sBlockName, BOOL bClear, BOOL bUnMask);
void hwBm9600WriteFoAcTimeslotSize(const bm9600InitParams_t *pInitParams,  uint32 nIndex, uint32 nTsLength);


/*
 *  hwBm9600NmIngressRankerWrite - write to NM block ingress Ranker table
 */
int hwBm9600NmIngressRankerWrite( const bm9600InitParams_t *pInitParams, uint32 uAddress, uint32 uData ) {
  int rv;

  rv = soc_bm9600_NmIngressRankerWrite(BM9600_BASE_ADDR, uAddress, uData);
  if (rv) {
      BM9600_ERR1("error(%d) returned from soc_bm9600_NmIngressRankerWrite()\n", rv);
  }
  return rv;
}

/*
 *  hwBm9600NmEgressRankerWrite - write to NM block egress Ranker table
 */
int hwBm9600NmEgressRankerWrite( const bm9600InitParams_t *pInitParams, uint32 uAddress, uint32 uData ) {
    int rv;

    rv = soc_bm9600_NmEgressRankerWrite(BM9600_BASE_ADDR, uAddress, uData);
    if (rv) {
	BM9600_ERR1("error(%d) returned from soc_bm9600_NmEgressRankerWrite()\n", rv);
    }
    return rv;
}

/*
 *  hwBm9600InaRandomNumGenWrite - write to Ina block random table
 */
int hwBm9600InaRandomNumGenWrite(const bm9600InitParams_t *pInitParams, uint32 uInstance, uint32 uAddress, uint32 uData ) {

    int rv;

    rv = soc_bm9600_InaRandomNumGenWrite(BM9600_BASE_ADDR, uAddress, uInstance, uData);

    if (rv) {
	BM9600_ERR1("InaRandomNumGenWrite Failed:  rv=%d\n", rv);
    }
    return rv;
}

/*
 *  hwBm9600NmRandomNumGenWrite - write to Nm block random table
 */
int hwBm9600NmRandomNumGenWrite(const bm9600InitParams_t *pInitParams,  uint32 uAddress, uint32 uData ) {

    int rv;
    rv = soc_bm9600_NmRandomNumGenWrite(BM9600_BASE_ADDR, uAddress,  uData);
    if (rv) {
	BM9600_ERR1("NmRandomNumGenWrite Failed:  rv=%d\n", rv);
    }
    return rv;
}

/*
 * Get/set a single ESET entry
 */
int hwBm9600EsetSet(int nUnit, uint32 uEset, uint64 uuLowNodesMask, uint32 uHiNodesMask, uint32 uMcFullEvalMin, uint32 uEsetFullStatusMode)
{
    sbZfFabBm9600NmEmtEntry_t eset_info;
    uint8                   data[(SB_ZF_FAB_BM9600_NMEMTENTRY_SIZE_IN_BYTES + 2)];
    uint32                    uSbRv;
    uint32                    uLowPhyNodesMask0 = 0;
    uint32                    uLowPhyNodesMask1 = 0;
    uint32                    uHiPhyNodesMask = 0;
    int                       node, phy_node;

    for (node = 0; node < 64; node++) {
        if (COMPILER_64_BITTEST(uuLowNodesMask, node)) {
            phy_node = SOC_SBX_L2P_NODE(nUnit, node);
            if (phy_node >= 64) {
                uHiPhyNodesMask |= (1 << (phy_node - 64));
            }
            else {
                if (phy_node < 32) {
                    uLowPhyNodesMask0 |= (1 << phy_node);
                }
                else {
                    uLowPhyNodesMask1 |= (1 << (phy_node-32));
                }
            }
        }
    }
    for (node = 64; node < 72; node++) {
        if (uHiNodesMask & (1 << (node - 64))) {
            phy_node = SOC_SBX_L2P_NODE(nUnit, node);
            if (phy_node >= 64) {
                uHiPhyNodesMask |= (1 << (phy_node - 64));
            }
            else {
                if (phy_node < 32) {
                    uLowPhyNodesMask0 |= (1 << phy_node);
                }
                else {
                    uLowPhyNodesMask1 |= (1 << (phy_node-32));
                }
            }
        }
    }

    eset_info.m_uEsetFullStatusMode = uEsetFullStatusMode;
    eset_info.m_uMfem = uMcFullEvalMin;
    COMPILER_64_SET(eset_info.m_uuEsetMember0, uLowPhyNodesMask1, uLowPhyNodesMask0); 
    eset_info.m_uEsetMember1 = uHiPhyNodesMask;
    sbZfFabBm9600NmEmtEntry_Pack(&eset_info, data,
                                 (SB_ZF_FAB_BM9600_NMEMTENTRY_SIZE_IN_BYTES + 2));

    uSbRv = soc_bm9600_NmEmtWrite(nUnit, uEset, *((uint32 *)(data + 0)),
                                  *((uint32 *)(data + 4)),
                                  *((uint32 *)(data + 8)));

    if( uSbRv ) {
        return(uSbRv);
    }

    return(uSbRv);
}


int hwBm9600EsetGet(int nUnit, uint32 uEset, uint64 *pLowNodesMask, uint32 *pHiNodesMask, uint32 *pMcFullEvalMin, uint32 *pEsetFullStatusMode)
{
    sbZfFabBm9600NmEmtEntry_t eset_info;
    uint8                   data[(SB_ZF_FAB_BM9600_NMEMTENTRY_SIZE_IN_BYTES + 2)];
    uint32                    uEsetData[3];
    uint32                    uSbRv;
    uint64                    uLowPhyNodesMask = COMPILER_64_INIT(0,0);
    uint32                    uHiPhyNodesMask = 0;
    int                       node, phy_node;


    uSbRv = soc_bm9600_NmEmtRead(nUnit, uEset, &uEsetData[0],
                                 &uEsetData[1], &uEsetData[2]);
    if (uSbRv) {
        return(uSbRv);
    }

    *((uint32 *)(data + 0)) = uEsetData[0];
    *((uint32 *)(data + 4)) = uEsetData[1];
    *((uint32 *)(data + 8)) = uEsetData[2];
    sbZfFabBm9600NmEmtEntry_Unpack(&eset_info, data,
                                   (SB_ZF_FAB_BM9600_NMEMTENTRY_SIZE_IN_BYTES + 2));

    uLowPhyNodesMask = eset_info.m_uuEsetMember0;
    uHiPhyNodesMask = eset_info.m_uEsetMember1;

    COMPILER_64_ZERO(*pLowNodesMask);
    *pHiNodesMask = 0;
    for (phy_node = 0;  phy_node < 64; phy_node++) {
        if (COMPILER_64_BITTEST(uLowPhyNodesMask,phy_node)) {
            node = SOC_SBX_P2L_NODE(nUnit, phy_node);
            if (node >= 64) {
                (*pHiNodesMask) |= (1 << (node - 64));
            }
            else {
                uint64 uuTmp = COMPILER_64_INIT(0,1);
                COMPILER_64_SHL(uuTmp, node);
                COMPILER_64_OR(*pLowNodesMask, uuTmp);
            }
        }
    }
    for (phy_node = 64; phy_node < 72; phy_node++) {
        if (uHiPhyNodesMask & (1 << (phy_node - 64))) {
            node = SOC_SBX_P2L_NODE(nUnit, phy_node);
            if (node >= 64) {
                (*pHiNodesMask) |= (1 << (node - 64));
            }
            else {
                uint64 uuTmp = COMPILER_64_INIT(0,1);
                COMPILER_64_SHL(uuTmp, node);
                COMPILER_64_OR(*pLowNodesMask, uuTmp);
            }
        }
    }

    *pMcFullEvalMin = eset_info.m_uMfem;
    *pEsetFullStatusMode = eset_info.m_uEsetFullStatusMode;

    return(uSbRv);
}




/*
 * hwBm9600InitSiStep0
 */
int hwBm9600InitSiStep0(int nNum, const bm9600InitParams_t *pInitParams){

    uint32 uSiDebug1;
    uint32 uSiSdPllCfg;
    uint32 uLaneMode;
    uint32 uSiSdConfig;
    int rv = SOC_E_NONE;
    
    /* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init si */
    if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	return SOC_E_NONE; 
    }

    BM9600_LOG1( "Bringing up Si%d Step0 \n", nNum);

    uSiDebug1 = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_DEBUG1);
    uSiDebug1 = SAND_HAL_MOD_FIELD(PL, SI_DEBUG1, RX_MSM_LBA_DEBUG, uSiDebug1, 1);
    SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_DEBUG1, uSiDebug1);


    if( (nNum % PL_SI_PER_HC) == 0 ) {

          /*  set SD_PLL.FORCE_SPEED_STRAP to set speed for 6.25
           * FNS 10/24/07 source Michael Lau/Hsin-Yuan Ho info
           * Input [5:0] force_speed_strap;
           * The following is the force_speed table mapping for the
           * strap and the MDIO register. (NOTE: The difference in the mapping.)
           * -----------------------------------------------
           * force_speed_strap ||| force_speed (reg) ||| Speed
           * -----------------------------------------------
           * 6'h00 ||| n/a use ieee regs ||| 10 Mbps
           * 6'h01 ||| n/a use ieee regs ||| 100 Mbps
           * 6'h02 ||| n/a use ieee regs ||| 1000 Mbps (sgmii)
           * 6'h02 ||| 6'h0x ||| 1000 Mbps (fiber)
           * 6'h03 ||| 6'h10 ||| 2500 Mbps
           * 6'h04 ||| 6'h11 ||| 5000 Mbps
           * 6'h05 ||| 6'h12 ||| 6000 Mbps
           * 6'h06 ||| 6'h13 ||| 10000 Mbps
           * 6'h07 ||| 6'h14 ||| 10000 Mbps (Cx4)
           * 6'h08 ||| 6'h15 ||| 12000 Mbps
           * 6'h09 ||| 6'h16 ||| 12500 Mbps (NOTE: Invalid speed - Set to 12G )
           * 6'h0A ||| 6'h17 ||| 13000 Mbps
           * 6'h0B ||| 6'h18 ||| 15000 Mbps
           * 6'h0C ||| 6'h19 ||| 16000 Mbps
           * 6'h0D ||| n/a ||| 1000BASE-KX
           * 6'h0E ||| n/a ||| 10GBASE-KX4
           * 6'h0F ||| n/a ||| 10GBASE-KR
           * 6'h10 ||| 6'h1A ||| 5000 Mbps single  <----------- our choice - after 8b10b decoding original 6.25G
           * 6'h11 ||| 6'h1B ||| 6364 Mbps single
           * 6'h12 ||| 6'h1C ||| 20000 Mbps
           * 6'h13 ||| 6'h1D ||| 25455 Mbps
           */
          uSiSdPllCfg = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_PLL_CONFIG);
          uSiSdPllCfg = SAND_HAL_MOD_FIELD(PL, SI_SD_PLL_CONFIG, FORCE_SPEED_STRAP, uSiSdPllCfg, 0x10);
          SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_PLL_CONFIG, uSiSdPllCfg);
    }

    /* All lanes */
    {
	/* The exception for SI_SD_PLL_CONFIG is the lane mode which is per lane and therefore per SI */
	uSiSdPllCfg = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_PLL_CONFIG);
	uLaneMode = PL_HC_LANE_MODE_HALF_SPEED;

	uSiSdPllCfg = SAND_HAL_MOD_FIELD(PL, SI_SD_PLL_CONFIG, LANE_MODE_STRAP, uSiSdPllCfg, uLaneMode);
	SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_PLL_CONFIG, uSiSdPllCfg);

	/* Sanity check configuration, QE2K nodes are single channel */
	if( (pInitParams->si[nNum].bEvenChannelOn == TRUE) &&
	    (pInitParams->si[nNum].bOddChannelOn == TRUE) &&
	    (pInitParams->si[nNum].eNodeType == QE2000) ) {
            BM9600_ERR( "Configuration error: Link is associated with a QE2K node but has both channels on");
            rv = SOC_E_CONFIG;
	}

	/* Enable 64B/66B encoding - note that when we do this we must do this before taking the lane out of reset
	 * UINT uSiSdConfig  = SAND_HAL_READ_STRIDE(m_nBaseAddr, PL, SI, nNum, SI_SD_CONFIG);
	 * uSiSdConfig  = SAND_HAL_MOD_FIELD(PL, SI_SD_CONFIG, ED66_DEF, uSiSdConfig, 1);
	 * SAND_HAL_WRITE_STRIDE(m_nBaseAddr, PL, SI, nNum, SI_SD_CONFIG, uSiSdConfig);
	 */

	uSiSdConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_CONFIG);
	/* ports enabled individually through hypercore via bcm_port_init, enable by default here */
	uSiSdConfig = SAND_HAL_MOD_FIELD (PL, SI_SD_CONFIG, RX_PWRDWN, uSiSdConfig, 0);
	uSiSdConfig = SAND_HAL_MOD_FIELD (PL, SI_SD_CONFIG, TX_PWRDWN, uSiSdConfig, 0);
        uSiSdConfig = SAND_HAL_MOD_FIELD (PL, SI_SD_CONFIG, TX_FIFO_RESET, uSiSdConfig, 0);

	SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_CONFIG, uSiSdConfig);
    }
    return rv;
}

/*
 * hwBm9600InitSiStep1
 */
int
hwBm9600InitSiStep1(int nNum, const bm9600InitParams_t *pInitParams){

    uint32 uSiSdReset;
    int rv = SOC_E_NONE;
    uint32 uSiPllStatus = 0;
    soc_timeout_t timeout;
    uint32 uAck = 0;

    /* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init si */
    if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	return SOC_E_NONE; 
    }

    BM9600_LOG1( "Bringing up Si%d Step1 \n", nNum);


    if( (nNum % PL_SI_PER_HC) == 0 ) {

	uSiSdReset = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_RESET);
	/*  deassert pwrdwn_pll */
	uSiSdReset = SAND_HAL_MOD_FIELD(PL, SI_SD_RESET, PWRDWN_PLL, uSiSdReset, 0);
	/*  deassert iddq and write back */
	uSiSdReset = SAND_HAL_MOD_FIELD(PL, SI_SD_RESET, IDDQ, uSiSdReset, 0);
	SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_RESET, uSiSdReset);
	
	/* deassert hw_reset and write back
	 */
	uSiSdReset = SAND_HAL_MOD_FIELD(PL, SI_SD_RESET, HW_RESET, uSiSdReset, 0);
	SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_RESET, uSiSdReset);
	/* deassert mdioregs_reset */
	uSiSdReset = SAND_HAL_MOD_FIELD(PL, SI_SD_RESET, MDIOREGS_RESET, uSiSdReset, 0);
	/* deassert pll_reset and write back */
	uSiSdReset = SAND_HAL_MOD_FIELD(PL, SI_SD_RESET, PLL_RESET, uSiSdReset, 0);
	SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_RESET, uSiSdReset);

	/* Check lock */
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    uSiPllStatus = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nNum, SI_SD_STATUS);
	    uAck = SAND_HAL_GET_FIELD(PL, SI_SD_STATUS, TX_PLL_LOCK, uSiPllStatus);
	}
	if ( !uAck ) {
	    BM9600_ERR1("Error - no PLL lock for SI%d\n", nNum);
	    rv = SOC_E_TIMEOUT;
	}
    }
    return rv;
}


/*
 * hwBm9600InitAi
 */
void
hwBm9600InitAi(int nNum, const bm9600InitParams_t *pInitParams){
    uint32 uAiConfig;

    if ( pInitParams->ai[nNum].bBringUp ) {
	if ( pInitParams->bAllowBroadcast != 1)  {
	    uAiConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, AI, nNum, AI_CONFIG);
	    uAiConfig = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, SOFT_RESET, uAiConfig, 0);
	    /* Bring AI out of reset even if not being enabled */
	    SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, AI, nNum, AI_CONFIG, uAiConfig);
	} else {
	    if ( nNum == 0 ) {
		uAiConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, AI, nNum, AI_CONFIG);
		uAiConfig = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, SOFT_RESET, uAiConfig, 0);
		/* Bring AI out of reset even if not being enabled */
		SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, AI, BM9600_BROADCAST_OFFSET, AI_CONFIG, uAiConfig);
	    }
	}

	if ( pInitParams->ai[nNum].bIsEnabled ) {
	    BM9600_LOG1( "Bringing up Ai %0d\n", nNum);

	    uAiConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, AI, nNum, AI_CONFIG);
	    /* - NOTE : Polaris(.pdf) spec rev 1.0 shows soft reset and */
	    /* - AI_CONFIG field writes happening in the same write transaction */
	    /* - set enable */
	    uAiConfig = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, ENABLE, uAiConfig, 1);
	    /* - write QE4k bit */
	    /* The SI init params already have */
	    /* - a flag indicating node type and SI0 is always connected to AI0, SI1 to AI1, etc */
	    /* - so just use the associated SI init param. */
	    if( pInitParams->si[nNum].eNodeType == QE2000 ) {
		uAiConfig = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, QE40_MODE, uAiConfig, 0);
		uAiConfig = SAND_HAL_MOD_FIELD( PL, AI_CONFIG, EXPANDED_QE2K_ESET_SPACE, uAiConfig,
						pInitParams->ai[nNum].bExpandedQe2kEsetSpace );

	    } else {
		uAiConfig = SAND_HAL_MOD_FIELD(PL, AI_CONFIG, QE40_MODE, uAiConfig, 1);
	    }
	    SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, AI, nNum, AI_CONFIG, uAiConfig);
	}
    }
}

/*
 * hwBm9600InitInaStep0
 */
int
hwBm9600InitInaStep0(int nNum, const bm9600InitParams_t *pInitParams){

    uint32 uInaConfig;
    uint32 uData;
    int rv = SOC_E_NONE;

    if ( pInitParams->ina[nNum].bBringUp ) {
	/*  Bring INA out of reset even if not being enabled */
	if ( nNum == 0 ) {
	    uInaConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG);
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, SOFT_RESET, uInaConfig, 0);
	    SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, BM9600_BROADCAST_OFFSET, INA_CONFIG, uInaConfig);
	}

	if ( pInitParams->ina[nNum].bIsEnabled ) {
	    BM9600_LOG1( "Bringing up Ina %0d Step0\n", nNum);

	    if (pInitParams->bEnableEcc == 0){
		BM9600_LOG1( "Backdoor allowed disable ECC/parity check for INA(%d)\n",nNum);
		/*SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, INA_DEBUG, ECC_PARITY_ENABLE, 0x0); */

		uData = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_DEBUG);
		uData = SAND_HAL_MOD_FIELD(PL, INA_DEBUG, ECC_PARITY_ENABLE, uData, 0x0);
		SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_DEBUG, uData);
	    }

	    /*  set init (no init_done in INA_CONFIG) */
	    uInaConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG);

            /* INA will be configured via bcm_stk_module_enable() API */
#ifndef _BM9600_INA_ENABLE_ON
            uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, uInaConfig, 0);
#else /* _BM9600_INA_ENABLE_ON */
            uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ENABLE, uInaConfig, 1);
#endif /* !(_BM9600_INA_ENABLE_ON) */
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, INIT, uInaConfig, 1);
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, CRIT_UPD_NCU_ENABLE, uInaConfig, pInitParams->ina[nNum].bCritUpdNcudEnable);
	    /* Enable rand_generator after the INA rand tables are initialized */
	    /* uInaConfig = SAND_HAL_MOD_FIELD(PL,INA_CONFIG,RAND_GEN_ENABLE,uInaConfig, pInitParams->ina[nNum].bRandGenEnable); */
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, ESET_LIMIT, uInaConfig, pInitParams->ina[nNum].uEsetLimit);
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, FILTER_PRI_UPDATES, uInaConfig, pInitParams->ina[nNum].bFilterPriUpdates);
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, MAXPRI_PRI, uInaConfig, pInitParams->ina[nNum].uMaxPriPri);
	    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, STROBE_DELAY, uInaConfig, pInitParams->ina[nNum].uStrobeDelay);

	    SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG, uInaConfig);

	    /* set INA_PRI_FULL_THRESH */
            if((pInitParams->si[nNum].eNodeType == QE4000) &&
               (!soc_bm9600_features(pInitParams->m_nBaseAddr, soc_feature_egr_independent_fc))) {
                
                uData = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_PRI_FULL_THRESH);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI1_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI2_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI3_FULL_THRESH, uData, 0x2);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI4_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI5_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI6_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI7_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI8_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI9_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI10_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI11_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI12_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI13_FULL_THRESH, uData, 0x1);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI14_FULL_THRESH, uData, 0x3);
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI15_FULL_THRESH, uData, 0x0);
                SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_PRI_FULL_THRESH, uData);
            } else {
                uData = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_PRI_FULL_THRESH);
                if (pInitParams->uSpMode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
                    uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI13_FULL_THRESH, uData, 0x1);
                }
                uData = SAND_HAL_MOD_FIELD(PL, INA_PRI_FULL_THRESH, PRI15_FULL_THRESH, uData, 0x0);
                SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_PRI_FULL_THRESH, uData);
            }

	    /* memory initialization on Portmap table */
	    /* memory initialization on Random Number table */
	}
    }
    return rv;
}

/*
 * hwBm9600InitInaStep1
 * this function had calls to generate a random number table.  now it is passed in
 *
 */
int
hwBm9600InitInaStep1(int nNum, const bm9600InitParams_t *pInitParams){

    uint32 uTableAddr;
    uint32 randVal,uValue = 0,uInaConfig;
    uint32 zInaSysPortMap = 0xFFF;
    uint32 uSysPort;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int rv = SOC_E_NONE;

    if ( pInitParams->ina[nNum].bBringUp ) {

	if ( pInitParams->ina[nNum].bIsEnabled ) {

	    BM9600_LOG1( "Bringing up Ina %d Step1\n", nNum);
	    if (pInitParams->nm.bInitRankerTables) {

		BM9600_LOG1( "Initialize RandomGen Memory for INA %d. \n", nNum);
		for ( uTableAddr = 0; uTableAddr < INA_RAND_NUM_TABLE_NUM_ENTRIES; uTableAddr++){
		    randVal = pInitParams->nm.pRandTable[uTableAddr];
		    rv = hwBm9600InaRandomNumGenWrite (pInitParams, nNum, uTableAddr, randVal);
		    if (rv) {
			return rv;
		    }
		}
	    }

            if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){
              for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
		uValue = SAND_HAL_READ_STRIDE_POLL(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG);
		uAck = SAND_HAL_GET_FIELD(PL, INA_CONFIG, INIT_DONE, uValue);
              }
              
              if ( !uAck ) {
                  BM9600_ERR1("Timeout waiting for INA %d Init Done.\n", nNum);
		rv = SOC_E_TIMEOUT;
		return rv;
              }
            }
	    {
		/*  write back to clear INIT_DONE */
		SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG, uValue);
		/*  Now do write to enable Randgen, otherwise it clears init_done bit and causes timeout error */
		uInaConfig = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG);
		/*  Enable rand_generator after the INA rand tables are initilaized */
		/*  Enable_random only if running selective tests */
		if (pInitParams->nm.bInitRankerTables) {
		    uInaConfig = SAND_HAL_MOD_FIELD(PL, INA_CONFIG, RAND_GEN_ENABLE, uInaConfig, pInitParams->ina[nNum].bRandGenEnable);
		}
		SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum, INA_CONFIG, uInaConfig);
	    }


            /* Polaris B0 specific configuration */
            if ( (SOC_PCI_REVISION(pInitParams->m_nBaseAddr) != BCM88130_A0_REV_ID) &&
                 (SOC_PCI_REVISION(pInitParams->m_nBaseAddr) != BCM88130_A1_REV_ID) ) {
		uValue = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum,
                                                                        INA_CONNECTION_PRI_BITMAP);
                if (pInitParams->uSpMode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
                    /* soc property "bcm_cosq_sp_mode=1" */
                    uValue |= (1 << (13 - 1)); /* HOLDPRI */
                    uValue |= (1 << (15 - 1)); /* MAXPRI */
                }
                else {
                    /* soc property "bcm_cosq_sp_mode=0" */
                    uValue |= (1 << (12 - 1)); /* HOLDPRI */
                    uValue |= (1 << (15 - 1)); /* MAXPRI */
                }
                SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, nNum,
                                                                INA_CONNECTION_PRI_BITMAP, uValue);
            }

	}
	if ( pInitParams->bInaSysportMap == 1 ) {
	    SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_DATA0, (uint32)zInaSysPortMap);
	    BM9600_LOG( "Broadcast writing the INA SysPort Map for all INAs");
	    for( uSysPort = 0; uSysPort < BM9600_MAX_NUM_SYSPORTS; uSysPort++ ) {
		SAND_HAL_WRITE_STRIDE( pInitParams->m_nBaseAddr, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_CTRL,
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,SELECT, 2) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,REQ, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, 1) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,RD_WR_N, 0) |
				       SAND_HAL_SET_FIELD(PL, INA_MEM_ACC_CTRL,ADDRESS, uSysPort));
		/*  Now wait for ACK */
		uAck = 0;
		for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
		    int nData = SAND_HAL_READ_STRIDE( pInitParams->m_nBaseAddr, PL, INA, BM9600_BROADCAST_OFFSET, INA_MEM_ACC_CTRL);
		    uAck = SAND_HAL_GET_FIELD(PL, INA_MEM_ACC_CTRL,ACK, nData);
		}
		if (!uAck) {
		    BM9600_ERR1("Error broadcast writing INA Sysport memory uAddress=0x%08x\n", uSysPort);
		    rv = SOC_E_TIMEOUT;
		    return rv;
		}
	    }
	}
    }
    return rv;
}

/*
 * hwBm9600InitFoStep0
 */
void hwBm9600InitFoStep0(const bm9600InitParams_t *pInitParams){

    uint32 i;
    uint32 uFoConfig0;
    uint32 uEnabledMask = 0;

    if ( pInitParams->fo.bBringUp == 0)
	return;

    BM9600_LOG( "Bringing up FO Step0\n");

    /*  set 0 on FO_CONFIG0.SOFT_RESET */
    uFoConfig0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, FO_CONFIG0);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, SOFT_RESET, uFoConfig0, 0);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG0, uFoConfig0);
    /* set FO_CONFIG0.LINK_ENABLE */
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG3, pInitParams->fo.uLinkEnable);
    /*  set FO_CONFIG0.(DEFAULT_BM_ID, FORCE_NULL_GRANT, */
    /*  USE_GLOBAL_LINK_ENABLE, MAX_DIS_LINKS, LINK_DIS_TIMER_ENABLE, */
    /*  AUTO_LINK_DIS_ENABLE, FAILOVER_TIMER_ENABLE, AUTO_FAILOVER_ENABLE, */
    /*  LOCAL_BM_ID, INIT) */

    /*  always force NULL grants by setting FORCE_NULL_GRANT. they will */
    /*  not begin until we set FO_CONFIG0::AC_ENABLE */

    /* enable of INA's will be done by the application (via bcm_stk_module_enable) */
    uEnabledMask = 0;
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG4, uEnabledMask);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG5, uEnabledMask);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG6, uEnabledMask);

    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, DEFAULT_BM_ID, uFoConfig0, pInitParams->fo.uDefaultBmId);
    /*  Always force null grants during bringup */
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, FORCE_NULL_GRANT, uFoConfig0, 1);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, USE_GLOBAL_LINK_ENABLE, uFoConfig0, pInitParams->fo.uUseGlobalLinkEnable);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uFoConfig0, pInitParams->fo.uMaxDisLinks);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, LINK_DIS_TIMER_ENABLE, uFoConfig0, pInitParams->fo.uLinkDisTimerEnable);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AUTO_LINK_DIS_ENABLE, uFoConfig0, pInitParams->fo.uAutoLinkDisEnable);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, FAILOVER_TIMER_ENABLE, uFoConfig0, pInitParams->fo.uFailoverTimerEnable);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AUTO_FAILOVER_ENABLE, uFoConfig0, pInitParams->fo.uAutoFailoverEnable);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, LOCAL_BM_ID, uFoConfig0, pInitParams->fo.uLocalBmId);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, INIT,       uFoConfig0, 1);

    BM9600_LOG1(  "Writing FO_CONFIG0 with 0x%08x\n", uFoConfig0 );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG0, uFoConfig0);

    BM9600_LOG1(  "Writing FO_CONFIG1 with 0x%08x\n", pInitParams->fo.uFailoverTimeoutPeriod );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG1, pInitParams->fo.uFailoverTimeoutPeriod);

    BM9600_LOG1(  "Writing FO_CONFIG2 with 0x%08x\n", pInitParams->fo.uLinkDisTimeoutPeriod );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG2, pInitParams->fo.uLinkDisTimeoutPeriod);

    /* set FO_CONFIG2.LINK_DIS_TIMEOUT_PERIOD */
    /* set FO_CONFIG(7..12).FORCE_LINK_SELECT */
    for( i = 7; i <= 12; i++ ){
	BM9600_LOG2(  "Writing FO_CONFIG%d with 0x%08x\n", i, pInitParams->fo.uForceLinkSelect[i-7] );
    }

    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG7, pInitParams->fo.uForceLinkSelect[0]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG8, pInitParams->fo.uForceLinkSelect[1]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG9, pInitParams->fo.uForceLinkSelect[2]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG10, pInitParams->fo.uForceLinkSelect[3]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG11, pInitParams->fo.uForceLinkSelect[4]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG12, pInitParams->fo.uForceLinkSelect[5]);

    /*  set FO_CONFIG(13..16).FORCE_LINK_ACTIVE */
    for( i = 13; i <= 16; i++ ){
	BM9600_LOG2(  "Writing FO_CONFIG%d with 0x%08x\n", i, pInitParams->fo.uForceLinkSelect[i-13] );
    }

    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG13, pInitParams->fo.uForceLinkActive[0]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG14, pInitParams->fo.uForceLinkActive[1]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG15, pInitParams->fo.uForceLinkActive[2]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG16, pInitParams->fo.uForceLinkActive[3]);

    BM9600_LOG1(  "Writing FO_CONFIG17 with 0x%08x\n", pInitParams->fo.uNumNullGrants );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG17, pInitParams->fo.uNumNullGrants);

    for (i = 0; i < BM9600_NUM_TIMESLOTSIZE; i++) {
	hwBm9600WriteFoAcTimeslotSize(pInitParams, i, pInitParams->fo.uAcTimeslotSize[i]);
    }
}

/*
 * hwBm9600InitFoStep1
 */
int
hwBm9600InitFoStep1(const bm9600InitParams_t *pInitParams){
    int i;
    uint32 uFoConfig18,uValue = 0;
    uint32 uFoConfig0;
    uint32 uPiInterruptMask;
    soc_timeout_t timeout;
    uint32 uAck = 0;


    int rv = SOC_E_NONE;

    if ( pInitParams->fo.bBringUp == 0)
        return rv;

    BM9600_LOG( "Bringing up FO Step1\n");

    if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){
      for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
        uValue = SAND_HAL_READ_POLL(pInitParams->m_nBaseAddr, PL, FO_CONFIG0);
	uAck =  SAND_HAL_GET_FIELD(PL, FO_CONFIG0, INIT_DONE, uValue);
        
      }
      if (!uAck) {
          BM9600_ERR("Timeout waiting for FO Init Done.\n");
	rv = SOC_E_TIMEOUT;
	return rv;
      }
      
    }
    /*  write back to clear INIT_DONE and set FO_ENABLE */
    uValue = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, FO_ENABLE, uValue, 1);
    
    BM9600_LOG1(  "Writing FO_CONFIG0 with 0x%08x\n", uValue );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG0, uValue);


    /* set FO_CONFIG18.AC_TS_TO_GRANT_OFFSET */
    uFoConfig18 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, FO_CONFIG18);
    uFoConfig18 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG18, AC_TS_TO_GRANT_OFFSET, uFoConfig18, pInitParams->fo.uAcTsToGrantOffset);
    uFoConfig18 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG18, AC_TS_TO_NM_TS_OFFSET, uFoConfig18, pInitParams->fo.uAcTsToNmTsOffset );
    BM9600_LOG1(  "Writing FO_CONFIG18 with 0x%08x\n", uFoConfig18 );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG18, uFoConfig18);
    
    for (i = 0; i < BM9600_NUM_TIMESLOTSIZE; i++) {
	hwBm9600WriteFoAcTimeslotSize(pInitParams, i, pInitParams->fo.uAcTimeslotSize[i]);
    }
    
    uFoConfig0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, FO_CONFIG0);
    /* set FO_CONFIG0.AC_ENABLE */
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AC_ENABLE, uFoConfig0, 1);
    BM9600_LOG1(  "Writing FO_CONFIG0 with 0x%08x\n", uFoConfig0 );
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG0, uFoConfig0);

    /*  and enable interrupts */
    BM9600_LOG( "Enabling interrupts\n");
    SAND_HAL_RMW_FIELD(pInitParams->m_nBaseAddr, PL, PI_UNIT_INTERRUPT9_MASK, FO_DISINT, 0);
    uPiInterruptMask = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_INTERRUPT_MASK);
    uPiInterruptMask &= ~(1<<9); /* enable the interrupt from subunit 9 */
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_INTERRUPT_MASK, uPiInterruptMask);
    
    return rv;
}


/*
 * hwBm9600InitBwStep0 - Resembles Petronius behavior
 * These formulae will work as long as the minimum round-trip time
 * from timeslot header transmission from BM9600 to timeslot header
 * reception at BM9600 is greater than 132nS.
 *
 * Note that these formulae are based on the minimum Timeslot size -
 * in a graceful degradation scenario the timeslot size would increase,
 * however this does not change the effective round-trip time from grant
 * to critical update, hence the offset values continue to work and do
 * not have to be adjusted.
 */
void
hwBm9600InitBwStep0(const bm9600InitParams_t *pInitParams){
    uint32 uTimeSlotSizeInClocks;
    uint32 uRxSotOffset;
    uint32 uQeCommandOffset;
    uint32 uMaxNodesInAiChain;
    uint32 uBwIngressesConfig;
    uint32 uBwLatencyConfig;
    uint32 uBwGroupConfig;
    uint32 uBwPortConfig;
    uint32 uBwTimingConfig;
    uint32 uBwDebugInterfaceCtrl;
    uint32 uBwMiscConfig;
    uint32 uWredDeadline = 0;
    uint32 uDemandDeadline = 0;
    uint32 uDemandTimeslots = 0;
    uint32 uAllocDeadline = 0;
    uint32 uWrapup = 0;
#if 0000
    uint32 nWrapup = 0;
    BOOL bSegmented = FALSE;
#endif
    uint32 uStageTime;
    uint32 uAllocateTimeslots;
    uint32 uRequestTimeSlotNum, uBagNum;
    uint32 uBwWredConfig;
    uint32 uBwDemandConfig;
    uint32 uBwAllocConfig;
    uint32 uBwEpochConfig;
    uint32 uLink;

    if ( pInitParams->bw.bBringUp ) {
	BM9600_LOG( "Bringing up BW Step0\n");

	/*
	 * BW_OFFSET.RXSOT = TSCyclesMin - 8; BW_OFFSET.CMND = TSCyclesMin -
	 * 35; BW_MODE.LATENCY = LatencyTS + 1;
	 *
	 * Where TSCyclesMin is the minimum Timeslot size (all links active)
	 * expressed in core clock cycles (i.e. Tts/Tcyc).
	 *
	 * LatencyTS is the expected REQ->RSP latency expressed in
	 * timeslots. This is zero if the RSP associated with a REQ is available
	 * after the CUPD20 message returned in response to the GRANT20 message
	 * immediately preceding the REQ.
	 *
	 */

	/*  Use define for number of nodes for now but this should be derived from */
	/*         some other source. */
	uTimeSlotSizeInClocks =  hwBm9600GetTimeSlotSizeInClocks(pInitParams);
	/* uint32 uRxSotOffset = uTimeSlotSizeInClocks - 8 + 1 ;  ab 102903 +1 is bug 1503/1506 */
	uRxSotOffset = uTimeSlotSizeInClocks - pInitParams->bw.uRxSotToBaaOffset;

	uQeCommandOffset = uTimeSlotSizeInClocks - 35;

	uMaxNodesInAiChain = 0;
	for (uLink = 0; uLink < SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; uLink++) {
	    if ( (pInitParams->ina[uLink].bIsEnabled) &&
		 ((uLink % 24) > uMaxNodesInAiChain) ) {
		uMaxNodesInAiChain = (uLink % 24);
	    }
	}
	uBwIngressesConfig = SAND_HAL_SET_FIELD(PL, BW_INGRESSES_CONFIG, INGRESSES, uMaxNodesInAiChain+1);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_INGRESSES_CONFIG, uBwIngressesConfig);

	uBwLatencyConfig = SAND_HAL_SET_FIELD(PL, BW_LATENCY_CONFIG, LATENCY, pInitParams->bw.uMaxResponseLatencyInTimeSlots );
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_LATENCY_CONFIG, uBwLatencyConfig);

	uBwGroupConfig = SAND_HAL_SET_FIELD(PL, BW_GROUP_CONFIG, NUM_GROUPS, (pInitParams->bw.uActiveVoqNum - 1));
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_GROUP_CONFIG, uBwGroupConfig);

	uBwPortConfig = SAND_HAL_SET_FIELD(PL, BW_PORT_CONFIG, NUM_PORTS, (pInitParams->bw.uActiveBagNum - 1) );
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_PORT_CONFIG, uBwPortConfig);

	uBwTimingConfig = SAND_HAL_SET_FIELD(PL, BW_TIMING_CONFIG, QE_COMMAND_OFFSET, uQeCommandOffset) |
	    SAND_HAL_SET_FIELD(PL, BW_TIMING_CONFIG, RXSOT_TO_BAA_OFFSET, uRxSotOffset);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_TIMING_CONFIG, uBwTimingConfig);

	uBwDebugInterfaceCtrl = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, BW_DEBUG_INTERFACE_CTRL);
	if ( pInitParams->bw.bAllocationScaleEnable == TRUE ) {
	    uBwDebugInterfaceCtrl = SAND_HAL_MOD_FIELD(PL, BW_DEBUG_INTERFACE_CTRL, ALLOC_DEMAND_SCALE_ENABLE, uBwDebugInterfaceCtrl, 2);
	}
	uBwDebugInterfaceCtrl = SAND_HAL_MOD_FIELD(PL, BW_DEBUG_INTERFACE_CTRL, FETCH_SWAP, uBwDebugInterfaceCtrl, pInitParams->bw.uFetchSwap);
	uBwDebugInterfaceCtrl = SAND_HAL_MOD_FIELD(PL, BW_DEBUG_INTERFACE_CTRL, DISTRIB_SWAP, uBwDebugInterfaceCtrl, pInitParams->bw.uDistribSwap);

	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_DEBUG_INTERFACE_CTRL, uBwDebugInterfaceCtrl);

	/*  write BW_MISC_CONFIG.SOFT_RESET */
	uBwMiscConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, BW_MISC_CONFIG);
	uBwMiscConfig = SAND_HAL_MOD_FIELD(PL, BW_MISC_CONFIG, SOFT_RESET, uBwMiscConfig, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_MISC_CONFIG, uBwMiscConfig);

	/* write BW_MISC_CONFIG.INIT for repository initialization */
	uBwMiscConfig = SAND_HAL_MOD_FIELD(PL, BW_MISC_CONFIG, INIT, uBwMiscConfig, 1);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_MISC_CONFIG, uBwMiscConfig);

	uWredDeadline = 0;
	uDemandDeadline = 0;
	uDemandTimeslots = 0;
	uAllocDeadline = 0;
	uWrapup = 0;
#if 0000
	nWrapup = 0;
	bSegmented = FALSE;
#endif

	if ( pInitParams->bw.bWredEnable ) {
	    /* WRED processes 2 VOQs per timeslot, it also requires three */
	    /* timeslots to prime the pipeline and three to drain it... If */
	    /* WRED has not completed by its deadline, it will wind down, */
	    /* if it has not completed within 8 timeslots of its deadline */
	    /* it will be squelched... */
#if 0000
	    if (bSegmented){
		uWredDeadline = /*prime_pipe=*/ k**6 +*/ (nTotalGroups/nNumSegments + 1)/2;
		uWrapup = 0xf;
	    }else
#endif
            {
		uWredDeadline = /*prime_pipe=*/6 + (pInitParams->bw.uActiveVoqNum+1)/2;
	    }
	    uBwWredConfig = SAND_HAL_SET_FIELD(PL, BW_WRED_CONFIG, WRAP_UP, uWrapup) |
		SAND_HAL_SET_FIELD(PL, BW_WRED_CONFIG, LAST, uWredDeadline);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_WRED_CONFIG, uBwWredConfig);

	    uWredDeadline += 8/*wred_kill=*/ + uWrapup;
	} else
	    uWredDeadline = 0;

	BM9600_LOG1("WRED Deadline: %d\n", uWredDeadline);

	if ( pInitParams->bw.bBandwidthAllocationEnable ) {
	    /* demand estimation is performed by the QEs in DMode. This */
	    /* deadline merely determines when the BME expects that the */
	    /* QEs will have completed their task. We set it greater than */
	    /* the nWredDeadline (arbitrarily). */

	    /* if wred is enabled, wred processing at Pt performs in */
	    /*            - parallel with demand calculation at Ka. */
	    /* If wred is not enabled, then cannot assume the demand calculation at Kamino */
	    /* overlaps with WRED messages. Calculate a new nWredDeadline to account for */
	    /* times taken up by Kamino.Qm in demand calculation. Demand calculation at */
	    /* Kamino is 2 passes * 3 clocks per queue, plus 2 timeslots for startup and */
	    /* completion slops. The number of queues participate in demand calculation */
	    /* is configurable by the "demand_qnum_max" field in a QM register. The */
	    /* value of this field is passed to petronius via an attribute. For backward */
	    /* compatibility with existing tests, the deadline for demand calculation */
	    /* is done only when the attribute "demand_qnum_max" is non zero */
	    if (!pInitParams->bw.bWredEnable ) {
		uDemandTimeslots = pInitParams->uDemandClks / uTimeSlotSizeInClocks;
		uDemandTimeslots++; /* round up 1 timeslot */
		BM9600_LOG1( "Allow Kamino %d + 2 timeslots to process demand", uDemandTimeslots);
	    }
	    uDemandDeadline = uWredDeadline + uDemandTimeslots + 2;
	    uBwDemandConfig = SAND_HAL_SET_FIELD(PL, BW_DEMAND_CONFIG, LAST, uDemandDeadline );
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_DEMAND_CONFIG, uBwDemandConfig);

	    for ( uBagNum = 0, uRequestTimeSlotNum = 0; uBagNum < pInitParams->bw.uActiveBagNum; ++uBagNum ) {
		/* Even if the number of voqs in a BAG is zero, the hw will still blow through 1 timeslot    */
		uRequestTimeSlotNum += (PL_MAX(1,pInitParams->bw.auVoqNumInBag[uBagNum]) + 1) / 2;
	    }

	    /* allocation runs at better than 2 groups per timeslot. */
	    /* allocation must have completed by its deadline or it will be */
	    /* squelched. allocation requires 16 timeslots to prime its */
	    /* pipeline and 16 to wind down... */
	    uStageTime = 0;
	    uWrapup = 0;
#if 0000
	    if ( bSegmented ){

		uStageTime = pInitParams->bw.uGroupsPerPort;
		/*  the +1 is for the new formula where the latency */
		nWrapup = (pInitParams->uBwModeLatency+1+3)*uStageTime;
		uAllocDeadline = uDemandDeadline  +
		    pInitParams->bw.uSegmentSize * uStageTime + 1 - (uStageTime-1);
		/*       ???  math is suspect - see nick's 715 test */

	    }else
#endif
            {
		/* For each port, there is an overhead of of (bw_mode_latency+2) timeslots, */
		/*            - after the overhead, up to 2 demand requests per timeslot */
		/* also need to consider timeslots required by Distribute as calculated in nAllocateTimeslots */
		uAllocateTimeslots = pInitParams->bw.uActiveBagNum * (2 + pInitParams->bw.uMaxResponseLatencyInTimeSlots);
		uAllocateTimeslots += uRequestTimeSlotNum;
		uAllocDeadline = uDemandDeadline + /*prime_pipe=*/32 + uAllocateTimeslots;
	    }

	    if (uAllocDeadline >= pInitParams->bw.uEpochLength) {
		uAllocDeadline = (pInitParams->bw.uEpochLength - 1);
	    }

	    uBwAllocConfig = SAND_HAL_SET_FIELD(PL, BW_ALLOC_CONFIG, STAGE_TIME, uStageTime) |
		SAND_HAL_SET_FIELD(PL, BW_ALLOC_CONFIG, WRAP_UP, uWrapup) |
		SAND_HAL_SET_FIELD(PL, BW_ALLOC_CONFIG, LAST, uAllocDeadline);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_ALLOC_CONFIG, uBwAllocConfig);

	    /* this is in the case that we have segmented */
	    /* we have to extend the wrapup */
#if 0000
	    if (bSegmented) 
#endif
            {
		uAllocDeadline += uWrapup + 4;
            }

	} else {
	    uDemandDeadline  = uWredDeadline;
	    uAllocDeadline = uDemandDeadline;
	}

	uBwEpochConfig = SAND_HAL_SET_FIELD(PL, BW_EPOCH_CONFIG, BW_MSG_ENABLE, pInitParams->bw.bBandwidthAllocationEnable) |
	    SAND_HAL_SET_FIELD(PL, BW_EPOCH_CONFIG, BW_ENABLE, pInitParams->bw.bBandwidthAllocationEnable) |
	    SAND_HAL_SET_FIELD(PL, BW_EPOCH_CONFIG, WRED_MSG_ENABLE, pInitParams->bw.bWredEnable) |
	    SAND_HAL_SET_FIELD(PL, BW_EPOCH_CONFIG, WRED_ENABLE, pInitParams->bw.bWredEnable) |
	    SAND_HAL_SET_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, pInitParams->bw.uEpochLength);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_EPOCH_CONFIG, uBwEpochConfig);
    }
}

int
hwBm9600WredTableUpdate(uint32 BaseAddr, uint32 addr, uint32 write_value)
{
    int rv = SOC_E_NONE;
    uint32 nData;
    soc_timeout_t timeout;
    uint32 uAck = 0;


    SAND_HAL_WRITE(BaseAddr, PL, BW_R1_MEM_ACC_DATA, write_value);

    SAND_HAL_WRITE(BaseAddr, PL, BW_R1_MEM_ACC_CTRL,
		   SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,SELECT, 11) |
		   SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,REQ, 1) |
		   SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ACK, 1) |
		   SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,RD_WR_N, 0) |
		   SAND_HAL_SET_FIELD(PL, BW_R1_MEM_ACC_CTRL,ADDRESS, addr) );

    for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {

	nData = SAND_HAL_READ(BaseAddr, PL, BW_R1_MEM_ACC_CTRL);
	uAck = SAND_HAL_GET_FIELD(PL, BW_R1_MEM_ACC_CTRL, ACK, nData);
    }

    if (!uAck) {
	BM9600_ERR2("%s:  Error writing Wred Table, addr(0x%x)\n", FUNCTION_NAME(), (uint32)addr);
	rv = SOC_E_TIMEOUT;
	return rv;
    }
    return(rv);
}

int
hwBm9600InitWredTemplates(const bm9600InitParams_t *pInitParams)
{
    int template, color, rv = SOC_E_NONE;
    uint32 addr;
    uint32 write_value = 0xFFFFFFFF;


    for (template = 0; template < 256; template++) {
	for (color = 0; color < 3; color++) {
	    addr = template * 8;
	    addr = addr + (color * 2);

	    rv = hwBm9600WredTableUpdate(pInitParams->m_nBaseAddr, addr, write_value);
	    if (rv != 0) {
		BM9600_ERR("Error updating WRED.\n");
		return rv;
	    }

	    rv = hwBm9600WredTableUpdate(pInitParams->m_nBaseAddr, (addr + 1), write_value);
	    if (rv != 0) {
		BM9600_ERR("Error updating WRED.\n");
		return rv;
	    }
	}
    }
    return rv;
}

/*
 *  hwBm9600InitBwStep1
 */
int
hwBm9600InitBwStep1(const bm9600InitParams_t *pInitParams){

    uint32 uBwSchedConfig;
    int rv = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;


    if ( pInitParams->bw.bBringUp ) {

	BM9600_LOG( "Bringing up BW Step1\n");

        if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){
          for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    uAck = SAND_HAL_GET_FIELD(PL, BW_MISC_CONFIG,INIT_DONE,SAND_HAL_READ_POLL(pInitParams->m_nBaseAddr, PL, BW_MISC_CONFIG) );
          }
          
          if ( !uAck ) {
              BM9600_ERR("Timeout waiting for BW Init Done.\n");
	    rv = SOC_E_TIMEOUT;
          } else {
	    /*  Init done asserted before timeout. Resume initialization of Bw */
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_MISC_CONFIG, SAND_HAL_SET_FIELD(PL, BW_MISC_CONFIG, INIT_DONE, 1) );
            
	    /* memory initialization on WRED Curves table done during provision in AddCos() method */
	    /* set BW_SCHED_CONFIG.SCHED_ENABLE */
	    uBwSchedConfig = SAND_HAL_SET_FIELD(PL, BW_SCHED_CONFIG, TAG_CHECK_ENABLE, pInitParams->bw.bTagCheckEnable) |
              SAND_HAL_SET_FIELD(PL, BW_SCHED_CONFIG, SCHED_ENABLE, 1);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_SCHED_CONFIG, uBwSchedConfig);

	    rv = hwBm9600InitWredTemplates(pInitParams);
	    if (rv) {
              return rv;
	    }
          }
	} /* warm boot */
    }
    return rv;
}

/*
 *  hwBm9600InitBwStep2
 */
int
hwBm9600InitBwStep2(const bm9600InitParams_t *pInitParams){
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int rv = SOC_E_NONE;

    if ( pInitParams->bw.bBringUp ) {

      BM9600_LOG( "Bringing up BW Step2\n");

      if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){
	for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
          uAck = SAND_HAL_GET_FIELD(PL, BW_EVENT, SCHED_ACTIVE, SAND_HAL_READ_POLL(pInitParams->m_nBaseAddr, PL, BW_EVENT) );
	}
	if ( !uAck && (!pInitParams->bw.bIgnoreTimeout)) {
            BM9600_ERR("Timeout waiting for BW sched_active.\n");
          rv = SOC_E_TIMEOUT;
	} else {
          SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, BW_EVENT, SAND_HAL_SET_FIELD(PL, BW_EVENT, SCHED_ACTIVE, 1) );
	}
      }
    }
    return rv;
}

/*
 *  hwBm9600InitNmStep0
 */
void hwBm9600InitNmStep0(const bm9600InitParams_t *pInitParams){
    uint32 uNmConfig = 0;

    if ( pInitParams->nm.bBringUp ) {

	/* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init nm */
	if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	    return; 
	}
	BM9600_LOG( "Bringing up NM Step 0\n");

	if (pInitParams->bEnableEcc == 0){
	    SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, NM_DEBUG_INTERFACE_CTRL, ECC_PARITY_ENABLE, 0x0);
	}

	/*  clear NM_CONFIG.SOFT_RESET */
	uNmConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, NM_CONFIG);
	uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, SOFT_RESET, uNmConfig, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_CONFIG, uNmConfig);

	/* set NM_MIRROR_CONFG0..23 */

	/* set NM_CONFIG 'OFFSET' fields */
	uNmConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, NM_CONFIG);
	/*  INIT bit does not work */
	uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, INIT, uNmConfig, 1);
	uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, TS_TO_ESET_SHIFT_OFFSET, uNmConfig, pInitParams->nm.uTsToEsetShiftOffset);
	uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, NM_FULL_SHIFT_OFFSET, uNmConfig, pInitParams->nm.uNmFullShiftOffset);
	uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, GRANT_TO_FULL_SHIFT_OFFSET, uNmConfig, pInitParams->nm.uGrantToFullShiftOffset);

       /* setup multicast independent ef/nef flow control */
        if (soc_bm9600_features(pInitParams->m_nBaseAddr, soc_feature_egr_multicast_independent_fc)) {
	    uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, MC_SINGLE_BIT_FULL_STATUS_MODE_EN, uNmConfig, 1);
        }
        else {
	    uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, MC_SINGLE_BIT_FULL_STATUS_MODE_EN, uNmConfig, 0);
        }

	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_CONFIG, uNmConfig);

	/* set NM_DUAL_GRANT_CONFIG */
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_DUAL_GRANT_CONFIG0, pInitParams->nm.uNmDualGrantConfig0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_DUAL_GRANT_CONFIG1, pInitParams->nm.uNmDualGrantConfig1);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_DUAL_GRANT_CONFIG2, pInitParams->nm.uNmDualGrantConfig2);

        /* setup independent ef/nef flow control */
        if (soc_bm9600_features(pInitParams->m_nBaseAddr, soc_feature_egr_independent_fc)) {
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_FULL_STATUS_CONFIG0, 0xFFFFFFFF);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_FULL_STATUS_CONFIG1, 0xFFFFFFFF);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_FULL_STATUS_CONFIG2, 0xFFFFFFFF);
        }
        else {
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_FULL_STATUS_CONFIG0, 0x00);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_FULL_STATUS_CONFIG1, 0x00);
	    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_FULL_STATUS_CONFIG2, 0x00);
        }
    }
}

/*
 *  hwBm9600InitNmStep1
 *
 *  see NM initialization section of spec (3.10 in rev 1.0)
 *  for steps required to complete initialization
 */
int
hwBm9600InitNmStep1(const bm9600InitParams_t *pInitParams){

    uint32 uNmTableAddr;
    uint32 uNmConfig = 0;
    int rv = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;

    if ( pInitParams->nm.bBringUp == 0)
        return rv;

    /* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init nm */
    if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
      return rv; 
    }

    BM9600_LOG( "Bringing up NM Step 1\n");

    /*  set NM_PORTSET_HEAD */
    /*  initialize Eset Member Table Memory */
    /*  initialize Portset Info Memory */
    /*  initialize System Port Array Memory */
    /*  initialize Portset Link Table Memory */
    /*  initialize Random Number Table Memory */
    /*  initialize Ingress Random Rank Table Memory */
    /*  initialize Egress Random Rank Table Memory */

    if (pInitParams->nm.bInitRankerTables) {
	BM9600_LOG( "Initialize RandomGen Memory for NM.\n");

	for (uNmTableAddr = 0; uNmTableAddr <NM_RAND_NUM_TABLE_NUM_ENTRIES; uNmTableAddr++){
	    rv = hwBm9600NmRandomNumGenWrite(pInitParams, uNmTableAddr, pInitParams->nm.pRandTable[uNmTableAddr]);
	    if (rv) {
		return rv;
	    }
	}
	/*
	 *  Init NM Egress ranker table
	 */

	BM9600_LOG( "Initialize NM Ingress Ranker Memory for NM. \n");
	for (uNmTableAddr = 0; uNmTableAddr <NM_INGRESS_RANKER_TABLE_NUM_ENTRIES; uNmTableAddr++){
	    rv = hwBm9600NmIngressRankerWrite(pInitParams, uNmTableAddr, pInitParams->nm.pIngressRankerTable[uNmTableAddr] );
	    if (rv) {
		return rv;
	    }
	}

	BM9600_LOG( "Initialize NM Egress Ranker Memory for NM. \n");
	for (uNmTableAddr = 0; uNmTableAddr <NM_EGRESS_RANKER_TABLE_NUM_ENTRIES; uNmTableAddr++){
	    rv = hwBm9600NmEgressRankerWrite(pInitParams, uNmTableAddr, pInitParams->nm.pEgressRankerTable[uNmTableAddr] );
	    if (rv) {
		return rv;
	    }
	}

    }
    if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){
      for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
        uNmConfig = SAND_HAL_READ_POLL(pInitParams->m_nBaseAddr, PL, NM_CONFIG);
        uAck = ( SAND_HAL_GET_FIELD(PL, NM_CONFIG, INIT_DONE, uNmConfig) );
      }
      if ( !uAck) {
          BM9600_ERR("Timeout waiting for NM Init Done.\n");
        rv = SOC_E_TIMEOUT;
        return rv;
      }
    }

   /*  write back to clear INIT_DONE */
   SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_CONFIG, uNmConfig);
   /*  Enable rand_generator after the NM random table and ranker tables are initilaized */
   /*  Now do write to enable Randgen, otherwise it clears init_done bit and causes timeout error */
   uNmConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, NM_CONFIG);
   /*  Enable_random only if running selective tests */
   if (pInitParams->nm.bInitRankerTables)
       uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, ENABLE_RANDOM, uNmConfig, pInitParams->nm.bRandGenEnable);

   uNmConfig = SAND_HAL_MOD_FIELD(PL, NM_CONFIG, ENABLE_ALL_EGRESS_NODES, uNmConfig, pInitParams->nm.bEnableAllEgress);
   SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_CONFIG, uNmConfig);

    return rv;
}

/*
 * hwBm9600InitPi
 */
int
hwBm9600InitPi(const bm9600InitParams_t *pInitParams){
    uint32 uPiConfig = 0;
    uint32 uPiCorePllConfig0 = 0;
    uint32 uPiRevision = 0;
    uint32 uIdentification;
    uint32 uRevision;
    const uint32 EXPECTED_IDENTIFICATION = 0x0480;
    uint32 uPiCorePllConfig4 = 0;
    uint32 uPiCorePllConfig2 = 0;
    uint32 uPiLcpll0Config0 = 0;
    uint32 uPiLcpll1Config0 = 0;
    uint32 uPiCorePllStatus = 0;
    uint32 nTimeOut = 10;
    uint32 uPiLcpll0Status = 0;
    uint32 uPiLcpll1Status = 0;
    uint32 uOtpConfig = 0;
    uint32 uOtpChpCfgStatus = 0;
    soc_timeout_t timeout;
    uint32 uAck = 0;
    int rv = SOC_E_NONE;

    if ( pInitParams->pi.bBringUp ) {
	BM9600_LOG( "Bringing up Pi\n");

	uPiCorePllConfig0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG0);
	uPiCorePllConfig0 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG0, GLOBAL_RESET, uPiCorePllConfig0, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG0, uPiCorePllConfig0);

	uPiRevision = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_REVISION);
	uIdentification = SAND_HAL_GET_FIELD(PL, PI_REVISION, IDENTIFICATION, uPiRevision);
        uRevision = SAND_HAL_GET_FIELD(PL, PI_REVISION, REVISION, uPiRevision);
	if (uIdentification != EXPECTED_IDENTIFICATION) {
	    BM9600_ERR2("Did not get PI_REVISION::IDENTIFICATION of 0x%08x (got 0x%08x)\n",
                        EXPECTED_IDENTIFICATION, uIdentification);
	    rv = SOC_E_BADID;
	    return rv;
	}

	uPiCorePllConfig0 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG0, P1DIV, uPiCorePllConfig0, 1);
	uPiCorePllConfig0 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG0, P2DIV, uPiCorePllConfig0, 1);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG0, uPiCorePllConfig0);

        switch (uRevision) {
        case BCM88130_A0_REV_ID:
        case BCM88130_A1_REV_ID:
            /*
             *  GNATS 22952: update PLL settings for BM9600 Ax versions --
             *  same speed but use different divider values.
             */
            uPiCorePllConfig4 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG4);
            uPiCorePllConfig4 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG4, NDIV_INT, uPiCorePllConfig4, 40);
            SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG4, uPiCorePllConfig4);

            uPiCorePllConfig2 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG2);
            uPiCorePllConfig2 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG2, M1DIV, uPiCorePllConfig2, 4);
            uPiCorePllConfig2 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG2, M2DIV, uPiCorePllConfig2, 5);
            SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG2, uPiCorePllConfig2);

            break;
        case BCM88130_B0_REV_ID:
        default:
            /*
             *  GNATS 36692: use default PLL values for other BM9600 versions
             */
            uPiCorePllConfig4 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG4);
            uPiCorePllConfig4 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG4, NDIV_INT, uPiCorePllConfig4, 80);
            SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG4, uPiCorePllConfig4);

            uPiCorePllConfig2 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG2);
            uPiCorePllConfig2 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG2, M1DIV, uPiCorePllConfig2, 8);
            uPiCorePllConfig2 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG2, M2DIV, uPiCorePllConfig2, 10);
            SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG2, uPiCorePllConfig2);
        }

	uPiLcpll0Config0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_LCPLL0_CONFIG0);
	uPiLcpll0Config0 = SAND_HAL_MOD_FIELD(PL, PI_LCPLL0_CONFIG0, RESET, uPiLcpll0Config0, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_LCPLL0_CONFIG0, uPiLcpll0Config0);

	uPiLcpll1Config0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_LCPLL1_CONFIG0);
	uPiLcpll1Config0 = SAND_HAL_MOD_FIELD(PL, PI_LCPLL1_CONFIG0, RESET, uPiLcpll1Config0, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_LCPLL1_CONFIG0, uPiLcpll1Config0);

        if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){

          for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	    uPiCorePllStatus = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_COREPLL_STATUS);
	    uAck = SAND_HAL_GET_FIELD(PL, PI_COREPLL_STATUS, LOCK, uPiCorePllStatus);
          }

          if ( !uAck) {
              BM9600_ERR("Timeout waiting for PllLock.\n");
	    rv = SOC_E_TIMEOUT;
	    return rv;
          }
        }

	uPiCorePllConfig0 = SAND_HAL_MOD_FIELD(PL, PI_COREPLL_CONFIG0, DIGITAL_RESET, uPiCorePllConfig0, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_COREPLL_CONFIG0, uPiCorePllConfig0);

	/*  write 0 to soft reset bit in PI_CONFIG to deassert */
	/*  core reset */
	uPiConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_CONFIG);
	uPiConfig = SAND_HAL_MOD_FIELD(PL, PI_CONFIG, SOFT_RESET, uPiConfig, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_CONFIG, uPiConfig);

	uPiLcpll0Status = 0;
	uAck = 0;

        if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){

          for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {	
	    uPiLcpll0Status = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_LCPLL0_STATUS);
	    uAck = SAND_HAL_GET_FIELD(PL, PI_LCPLL0_STATUS, LOCK, uPiLcpll0Status);
          }

          if ( !uAck ) {
              BM9600_ERR("Timeout waiting for PiLcpll0 lock.\n");
	    rv = SOC_E_TIMEOUT;
	    return rv;
          }
        }


	uAck = 0;
        if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){

          for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) { 
	    uPiLcpll1Status = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_LCPLL1_STATUS);
	    uAck = SAND_HAL_GET_FIELD(PL, PI_LCPLL1_STATUS, LOCK, uPiLcpll1Status);
          }

          if ( !uAck ) {
              BM9600_ERR("Timeout waiting for PiLcpll1 lock.\n");
	    rv = SOC_E_TIMEOUT;
	    return rv;
          }
        }

	uOtpConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, OTP_CONFIG0);
	uOtpConfig = SAND_HAL_MOD_FIELD(PL, OTP_CONFIG0, SOFT_RESET, uOtpConfig, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, OTP_CONFIG0, uOtpConfig);


	uAck = 0;
        if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){

          for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) { 
	    uOtpChpCfgStatus = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, OTP_CHIP_CONFIG_STATUS);
	    uAck = SAND_HAL_GET_FIELD(PL, OTP_CHIP_CONFIG_STATUS, BISR_DONE, uOtpChpCfgStatus);
	    nTimeOut--;
	    if ( nTimeOut <= 0 ) {
              break;
	    }
          }

          if ( nTimeOut <= 0 ) {
              BM9600_ERR("Timeout waiting for BISR Done.\n");
	    rv = SOC_E_TIMEOUT;
	    return rv;
          }
        }

	/*  set poll time to 0 to keep the run times short */
	BM9600_LOG( "Putting PI INT_POLL_PERIOD to a fast poll rate\n");
	SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, PI_CONFIG, INT_POLL_PERIOD, 0x0);

	/*  speed up mdio clock */
	BM9600_LOG( "Setting MDIO clock to fast rate\n");
	SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, PI_CMIC_RATE_ADJUST_INT_MDIO, DIVISOR, 0x4);

	/* SDK-33154 disable nm/fo/bw clock if in crossbar only mode, else enable */
	/* nm is disabled in crossbar only mode                                   */
        if (pInitParams->nm.bBringUp == FALSE) {
          SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, PI_CLOCK_GATE_CONFIG2, NM_FO_BW_CLK_ENABLE, 0);
        } else {
          SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, PI_CLOCK_GATE_CONFIG2, NM_FO_BW_CLK_ENABLE, 1);
        }
    }
    return rv;
}

/*
 * hwBm9600InitXbStep0
 */
void hwBm9600InitXbStep0(const bm9600InitParams_t *pInitParams){
    uint32 uXbConfig0;

    if ( pInitParams->xb.bBringUp ) {

	/* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init xbar */
	if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	    return; 
	}

	BM9600_LOG( "Bringing up XB Step0\n");

	/* disable ecc */
	if (pInitParams->bEnableEcc == 0){
	    SAND_HAL_RMW_FIELD(BM9600_BASE_ADDR, PL, XB_ECC_STATUS, ECC_PARITY_ENABLE, 0x0);
	}

	uXbConfig0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, XB_CONFIG0);
	uXbConfig0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, XCFG_MODE, uXbConfig0, pInitParams->xb.eXcfgMode);
	uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, SOFT_RESET, uXbConfig0, 0);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_CONFIG0, uXbConfig0);

	if ( pInitParams->xb.eXcfgMode == XCFG_SE_MODE) {

          /* set SE Mode */
	    if (pInitParams->xb.bEnableSotPolicing == TRUE) {
		uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, SOT_POLICING_ENABLE, uXbConfig0, 1);
	    } else {
		uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, SOT_POLICING_ENABLE, uXbConfig0, 0);
	    }
	    uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, NO_DESKEW, uXbConfig0, 0);
	    uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, EPRT_XCFG_REPLACE_ENABLE, uXbConfig0, pInitParams->xb.bEnableXcfgReplace);
	}
	else {
	    /*  LCM Modes */
	    uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, SOT_POLICING_ENABLE, uXbConfig0, 0);
	    uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, NO_DESKEW, uXbConfig0, 1);
	    uXbConfig0 =  SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, EPRT_XCFG_REPLACE_ENABLE, uXbConfig0, 0);
	}

	uXbConfig0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, TS_JITTER_TOL, uXbConfig0, pInitParams->xb.uTsJitterTolerance);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_CONFIG0, uXbConfig0);

	uXbConfig0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, INIT_MODE, uXbConfig0, pInitParams->xb.uInitMode);
	uXbConfig0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, XCFG_BYPASS, uXbConfig0, pInitParams->xb.bBypass);
	uXbConfig0 = SAND_HAL_MOD_FIELD(PL, XB_CONFIG0, INIT, uXbConfig0, 1);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_CONFIG0, uXbConfig0);
    }
}

/*
 * hwBm9600InitXbStep1
 */
int
hwBm9600InitXbStep1(const bm9600InitParams_t *pInitParams)
{
    const uint32 NUM_IPRT_ENABLE_REGS = BM9600_NUM_LINKS / 16;
    uint32 uIprtEnables[BM9600_NUM_LINKS / 16];
    uint32 uIprtEnableIndex=0;
    uint32 uValue = 0;
    const uint32 NUM_EPRTS = (BM9600_NUM_LINKS * BM9600_CHANNELS_PER_LINK);
    const uint32 NUM_EPRTS_PER_REG = 8; /* 4b each entry */
    const uint32 NUM_EPRT_PASSTHRU_REGS = NUM_EPRTS / NUM_EPRTS_PER_REG;
    uint32 uReg;
    uint32 uPort;
    uint32 uEprtPassThru[(BM9600_NUM_LINKS * BM9600_CHANNELS_PER_LINK)/8]; /*NUM_EPRT_PASSTHRU_REGS*/ 
    /* uint32 uFailoverMode = (pInitParams->fo.uAutoLinkDisEnable << 1) | pInitParams->fo.uUseGlobalLinkEnable; */
    uint32 uPassThruAmount = 6; /*(uFailoverMode == 2) ? pInitParams->xb.uPassThru : 6; */
    int rv = SOC_E_NONE;
    soc_timeout_t timeout;
    uint32 uAck = 0;


    if ( pInitParams->xb.bBringUp == 0 )
        return rv;

    /* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init xbar */
    if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	return rv; 
    }

    BM9600_LOG("Bringing up XB Step1\n");
    if (!SOC_WARM_BOOT((int)pInitParams->m_nBaseAddr)){
      for (soc_timeout_init(&timeout, HW_BM9600_TIMEOUT_GENERAL, HW_BM9600_POLL_GENERAL); !soc_tightdelay_timeout_check(&timeout) && !uAck;) {
	uValue = SAND_HAL_READ_POLL(pInitParams->m_nBaseAddr, PL, XB_CONFIG0);
	uAck = SAND_HAL_GET_FIELD(PL, XB_CONFIG0, INIT_DONE, uValue);
      }
      if (!uAck) {
          BM9600_ERR("Timeout waiting for XB Init Done.\n");
	rv = SOC_E_TIMEOUT;
	return rv;
      }
    }
    /*  write back to clear INIT_DONE */
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_CONFIG0, uValue);
    
    /*  if HW REMAP table initialization is not acceptable : */
    /*  1. manually initialize XB_XCFG_REMAP_MEM */
    /*  2. set XB_XCFG_REMAP_MEM_ACC_CTRL */
    /*  3. set XB_XCFG_MEM_ACC_DATA */
    /*  optionally, program XB_EPRT_PASSTHRU_SIZE.PASSTHRU_SIZE */

    if (SOC_SBX_CFG(pInitParams->m_nBaseAddr)->diag_qe_revid != BCM88230_A0_REV_ID) {
	/* uPassThruAmount = 2; hardware team believe 2 should work
	 * but somehow it will lead to serdes time alignment issues
	 */
	uPassThruAmount = 6;
    }

    for (uReg = 0; uReg < NUM_EPRT_PASSTHRU_REGS; uReg++ ) {
	uEprtPassThru[uReg] = 0;
	for (uPort = 0; uPort < NUM_EPRTS_PER_REG; uPort++ ) {
	    uEprtPassThru[uReg] |= ((uPassThruAmount& 0xF) << (uPort*4));
	}
    }
    
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE0, uEprtPassThru[0]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE1, uEprtPassThru[1]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE2, uEprtPassThru[2]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE3, uEprtPassThru[3]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE4, uEprtPassThru[4]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE5, uEprtPassThru[5]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE6, uEprtPassThru[6]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE7, uEprtPassThru[7]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE8, uEprtPassThru[8]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE9, uEprtPassThru[9]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE10, uEprtPassThru[10]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE11, uEprtPassThru[11]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE12, uEprtPassThru[12]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE13, uEprtPassThru[13]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE14, uEprtPassThru[14]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE15, uEprtPassThru[15]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE16, uEprtPassThru[16]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE17, uEprtPassThru[17]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE18, uEprtPassThru[18]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE19, uEprtPassThru[19]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE20, uEprtPassThru[20]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE21, uEprtPassThru[21]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE22, uEprtPassThru[22]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_EPRT_PASSTHRU_SIZE23, uEprtPassThru[23]);
    
    /* set XB_IPRT_EN(0..5) */
    ASSERT(NUM_IPRT_ENABLE_REGS==6);

    /* each IPRT_ENABLE register contains two bits per link. */
    /* Disable all for now, will enable when port is enabled */
    for (uIprtEnableIndex=0; uIprtEnableIndex<NUM_IPRT_ENABLE_REGS; uIprtEnableIndex++ ) {
	uIprtEnables[uIprtEnableIndex] = 0;
    }
    
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_IPRT_ENABLE0, uIprtEnables[0]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_IPRT_ENABLE1, uIprtEnables[1]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_IPRT_ENABLE2, uIprtEnables[2]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_IPRT_ENABLE3, uIprtEnables[3]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_IPRT_ENABLE4, uIprtEnables[4]);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, XB_IPRT_ENABLE5, uIprtEnables[5]);
    
    return rv;
}

/*
 * Cleanup functions - Each block checks for errors
 */
void hwBm9600TraceErrorRegs(char *sBlockName, BOOL bClear, BOOL bUnMask )
{
  /* dummy for now ... */
}

void CleanUpSi(int nSi, const bm9600InitParams_t *pInitParams) {
  if (( pInitParams->si[nSi].bBringUp ) &&
      ( pInitParams->si[nSi].bIsEnabled ) ) {
      BM9600_LOG1("Clean up SI%d.\n", nSi);
    hwBm9600TraceErrorRegs("si", TRUE, TRUE);
  }
}

void CleanUpAi(int nAi, const bm9600InitParams_t *pInitParams) {
  if (( pInitParams->ai[nAi].bBringUp ) &&
      ( pInitParams->ai[nAi].bIsEnabled) ) {
      BM9600_LOG1("Clean up AI%d.\n", nAi);
    hwBm9600TraceErrorRegs("ai", TRUE, TRUE);
  }
}

void CleanUpIna(int nIna, const bm9600InitParams_t *pInitParams) {
  if (( pInitParams->ina[nIna].bBringUp ) &&
      ( pInitParams->ina[nIna].bIsEnabled ) ) {
      BM9600_LOG1("Clean up INA%d.\n", nIna);
    hwBm9600TraceErrorRegs("ina", TRUE, TRUE);
  }
}

void CleanUpFo(const bm9600InitParams_t *pInitParams) {
    uint32 uFoConfig0;

    if ( pInitParams->fo.bBringUp  ) {
        BM9600_LOG( "Clean up FO.\n");
      hwBm9600TraceErrorRegs("fo", TRUE, TRUE);
    }
    uFoConfig0 = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, FO_CONFIG0);
    uFoConfig0 = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, FORCE_NULL_GRANT, uFoConfig0, pInitParams->fo.uForceNullGrant);
    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, FO_CONFIG0, uFoConfig0);
}

void CleanUpBw(const bm9600InitParams_t *pInitParams) {
  if ( pInitParams->bw.bBringUp ) {
      BM9600_LOG( "Clean up BW.\n");
    hwBm9600TraceErrorRegs("bw", TRUE, TRUE);
  }
}

void CleanUpNm(const bm9600InitParams_t *pInitParams) {
  if ( pInitParams->nm.bBringUp ) {
      BM9600_LOG( "Clean up NM.\n");
    hwBm9600TraceErrorRegs("nm", TRUE, TRUE);
  }
}

void CleanUpPi(const bm9600InitParams_t *pInitParams) {
  if ( pInitParams->pi.bBringUp ) {
      BM9600_LOG( "Clean up PI.\n");
    hwBm9600TraceErrorRegs("pi", TRUE, TRUE);

    SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_INTERRUPT_MASK, 0);
  }
}

void CleanUpXb(const bm9600InitParams_t *pInitParams) {
  if ( pInitParams->xb.bBringUp ) {
      BM9600_LOG( "Clean up XB.\n");
    hwBm9600TraceErrorRegs("xb", TRUE, TRUE);
  }
}


/*
 * hwBm9600InitStep0 - First call to bring up all blocks in proper order
 */
int
hwBm9600InitStep0(const bm9600InitParams_t *pInitParams) {

    int nIna = 0;
    int uPiConfig;
    int rv = SOC_E_NONE;

    /* soft reset the chip first */
    if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == FALSE) {
	uPiConfig = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_CONFIG);
	uPiConfig = SAND_HAL_MOD_FIELD(PL, PI_CONFIG, SOFT_RESET, uPiConfig, 1);
	SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, PI_CONFIG, uPiConfig);

	rv = hwBm9600InitPi(pInitParams);
	if (rv) {
	    return rv;
	}
    }

    hwBm9600InitBwStep0(pInitParams);
    hwBm9600InitFoStep0(pInitParams);
    hwBm9600InitXbStep0(pInitParams);


    for ( nIna = 0; nIna < BM9600_NUMBER_OF_AI_INA; nIna++ ) {
      rv = hwBm9600InitInaStep0(nIna, pInitParams);
      if (rv) {
	  return rv;
      }
    }

    hwBm9600InitNmStep0(pInitParams);
    rv = hwBm9600InitNmStep1(pInitParams);
    if (rv) {
	return rv;
    }

    return rv;
}

/*
 * hwBm9600InitStep1 - Second call to bring up all blocks in proper order
 */
int
hwBm9600InitStep1(const bm9600InitParams_t *pInitParams) {

    int nIna = 0;
    int nAi = 0;
    int nSi = 0;
    uint32 uQuadEnableDoneMask = 0;
    uint32 uBaseSiInQuad;
    uint32 uSiConfig2;
    int rv;


    for ( nIna = 0; nIna < BM9600_NUMBER_OF_AI_INA; nIna++ ) {
	rv = hwBm9600InitInaStep1(nIna, pInitParams);
	if (rv) {
	    return rv;
	}
    }

    for ( nAi = 0; nAi < BM9600_NUMBER_OF_AI_INA; nAi++ ) {
	hwBm9600InitAi(nAi, pInitParams);
    }

    rv = hwBm9600InitBwStep1(pInitParams);
    if (rv) {
	return rv;
    }

    rv = hwBm9600InitFoStep1(pInitParams);
    if (rv) {
	return rv;
    }

    rv = hwBm9600InitXbStep1(pInitParams);
    if (rv) {
	return rv;
    }

    rv = hwBm9600InitBwStep2(pInitParams);
    if (rv) {
	return rv;
    }


    /*  This flag marks whether the global controls for HC associated  */
    /*  with the current Si lane have been set. */

    for ( nSi = 0; nSi < BM9600_NUMBER_OF_SI; nSi++ ) {

        if ( (pInitParams->si[nSi].bBringUp) &&
             (pInitParams->si[nSi].bIsEnabled) ) {

	    rv = hwBm9600InitSiStep0(nSi, pInitParams);
	    if (rv) {
		return rv;
	    }
        }
    }


    for ( nSi = 0; nSi < BM9600_NUMBER_OF_SI; nSi++ ) {

	/*  We're working on a new HC quad so reset the enable done flag. */
        if (nSi%4 == 0) {
	    uQuadEnableDoneMask = 0;
        }

        if ( (pInitParams->si[nSi].bBringUp) &&
             (pInitParams->si[nSi].bIsEnabled) ) {

	    uBaseSiInQuad = nSi - (nSi%4);

	    if ((uQuadEnableDoneMask & 1) == 0) {
		/* Initialize base in quad */
		rv = hwBm9600InitSiStep1(uBaseSiInQuad, pInitParams);
                if (rv) {
                  return rv;
                }
		uQuadEnableDoneMask |= 1;
	    }

        }
    }

    /* Set up si link error thresholds */
    for ( nSi = 0; nSi < BM9600_NUMBER_OF_SI; nSi++ ) {
	uSiConfig2 = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nSi, SI_CONFIG2);
	uSiConfig2 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG2, LS_ERR_WINDOW, uSiConfig2, pInitParams->uSiLsWindow);
	uSiConfig2 = SAND_HAL_MOD_FIELD(PL, SI_CONFIG2, LS_ERR_THRESH, uSiConfig2, pInitParams->uSiLsThreshold);
	SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, nSi, SI_CONFIG2, uSiConfig2);
    }

    /* Hypercore bringup and addtional SI bringup are moved to bcm port init */

    return HW_BM9600_STATUS_OK;
}

/*
 * hwBm9600InitStep2 - Init BM sysport tables
 *      set the sysport mapping table to 0 for each ina
 *      set port priority table to 0/0 for every enabled ina
 *      set sysport array to 0xFFF for each sysport in nm
 *      set portset link table to index/0xFF for each portset in nm
 *      set portset info table to 0 for each portset in nm
 *      set portset link head to portset 0
 */
uint32
hwBm9600InitStep2(const bm9600InitParams_t *pInitParams)
{
    int bSuccess;
    int portset, sysport;
    int nIna;

    if (!pInitParams->nm.bBringUp) {
      return HW_BM9600_STATUS_OK;
    }

    /* Init system port array to 0xFFF for every sysport */
    for (portset = 0; portset < BM9600_MAX_PORTSETS; portset++) {
	/* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init nm */
	if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	    break; 
	}
        bSuccess = soc_bm9600_NmSysportArrayWrite(pInitParams->m_nBaseAddr, portset, 0xFFFFFFFF, 0xFFFFFFFF,
                                                  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
        if (bSuccess != SOC_E_NONE) {
            BM9600_ERR2("ERROR: %s, Failed to init NM Sysport Array Memory row %d. \n",
                     FUNCTION_NAME(), (uint32) portset);
            return bSuccess;
        }
    }

    /* Init sysport maping table */
    for (nIna = 0; nIna < BM9600_NUM_LINKS; nIna++) {
      if (pInitParams->ina[nIna].bIsEnabled) {
        for (sysport = 0; sysport < BM9600_MAX_NUM_SYSPORTS; sysport++) {
          bSuccess = soc_bm9600_InaSysportMapWrite(pInitParams->m_nBaseAddr, sysport, nIna, 0);
          if (bSuccess != SOC_E_NONE) {
            BM9600_ERR2("ERROR: %s, Failed to init NM Sysport mapping table for sysport %d. \n",
                       FUNCTION_NAME(), (uint32) sysport);
            return bSuccess;
          }
        }
      }
    }

    /* Init porset link table to index/0xFF */
    for (portset = 0; portset < BM9600_MAX_PORTSETS; portset++) {
	/* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init nm */
	if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	    break; 
	}
        bSuccess = soc_bm9600_portset_link_table_write(pInitParams->m_nBaseAddr, portset, 0xFF);
        if (bSuccess != SOC_E_NONE) {
            BM9600_ERR2("ERROR: %s, Failed to init portset link table row %d. \n",
                     FUNCTION_NAME(), (uint32) portset);
            return bSuccess;
        }
    }

    /* Init portset info table to 0 */
    for (portset = 0; portset < BM9600_MAX_PORTSETS; portset++) {
	/* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init nm */
	if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == TRUE) {
	    break; 
	}
        bSuccess = soc_bm9600_portset_info_table_write(pInitParams->m_nBaseAddr, portset, 0, 0, 0, 0);
        if (bSuccess != SOC_E_NONE) {
            BM9600_ERR2("ERROR: %s, Failed to init portset info table row %d. \n",
                     FUNCTION_NAME(), (uint32) portset);
            return bSuccess;
        }
    }

    /* Init port priority table to 0/0 for all sysports
     * this would be done each time a new portset is allocated
     */
    for ( nIna = 0; nIna < BM9600_NUMBER_OF_AI_INA; nIna++ ) {
	bSuccess = soc_bm9600_InaEsetPriClear(pInitParams->m_nBaseAddr, nIna);
        if (bSuccess != SOC_E_NONE) {
            BM9600_ERR2("ERROR: %s, Failed to init Eset priority table for ina %d. \n",
                     FUNCTION_NAME(), (uint32) nIna);
            return bSuccess;
        }
    }

    /* If we are reconfiguring as an arbiter from crossbar or vice versa, don't re-init nm */
    if (SOC_SBX_CFG_BM9600(pInitParams->m_nBaseAddr)->bElectArbiterReconfig == FALSE) {
      /* Point the portset head to dummy entry 0 */
      SAND_HAL_WRITE(pInitParams->m_nBaseAddr, PL, NM_PORTSET_HEAD, 0);
    }
    return HW_BM9600_STATUS_OK;
}

/*
 * CleanUp - Clean up all blocks
 */
void CleanUp( bm9600InitParams_t *pInitParams) {

    int nAi = 0;
    int nIna = 0;
    uint32 uSiError;
    
    COMPILER_REFERENCE(uSiError);
    BM9600_LOG( "Broadcast Clean up all SI.\n");
      /* Clearing through broadcast, write all ones first to clear W1TC */
      /* then read to clear any COR_CTR fields */
      SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, BM9600_BROADCAST_OFFSET, SI_ERROR0, 0xFFFFFFFF);
      uSiError = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, BM9600_BROADCAST_OFFSET, SI_ERROR0);
      SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, BM9600_BROADCAST_OFFSET, SI_ERROR1, 0xFFFFFFFF);
      uSiError = SAND_HAL_READ_STRIDE(pInitParams->m_nBaseAddr, PL, SI, BM9600_BROADCAST_OFFSET, SI_ERROR1);
      /* Now unmask them all */
      SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, BM9600_BROADCAST_OFFSET, SI_ERROR0_MASK, 0);
      SAND_HAL_WRITE_STRIDE(pInitParams->m_nBaseAddr, PL, SI, BM9600_BROADCAST_OFFSET, SI_ERROR1_MASK, 0);

    for ( nAi = 0; nAi < BM9600_NUMBER_OF_AI_INA; nAi++ ) {
      CleanUpAi(nAi, pInitParams);
    }

    for ( nIna = 0; nIna < BM9600_NUMBER_OF_AI_INA; nIna++ ) {
      CleanUpIna(nIna, pInitParams);
    }

   CleanUpFo(pInitParams);
   CleanUpBw(pInitParams);
   CleanUpNm(pInitParams);
   CleanUpPi(pInitParams);
   CleanUpXb(pInitParams);
}

/*
 * hwBm9600Init  - Calls sequence of bringup steps above,
 *                 then calls Cleanup to check for errors.
 */
uint32
hwBm9600Init(bm9600InitParams_t *pInitParams)
{
    uint32 uTimeslotSizeNs;
    uint32 uBitPosition    = 0;
    int nNumBitsEnabled  = 0;
    uint32 rv = SOC_E_NONE;

    BM9600_LOG("hwBm9600Init called  \n");
    pInitParams->bEnableEcc = 0;

    rv = hwBm9600InitStep0(pInitParams);
    if (rv) {
	return rv;
    }

    if ( pInitParams->fo.uLocalBmId != pInitParams->fo.uDefaultBmId ) {
	/* get timeslot size from 'fo.uAcTimeslotSize' */
	for (uBitPosition = 0; uBitPosition < 32; uBitPosition++) {
	    if (pInitParams->fo.uLinkEnable & (1 << uBitPosition) ) {
                nNumBitsEnabled++;
            }
        }
        /* select 1 of 32 Ac Timeslot size registers using 'enabled link count' - 1 */
        nNumBitsEnabled--;
        if ( nNumBitsEnabled < 0 ) {
            nNumBitsEnabled = 31;
        }
        uTimeslotSizeNs = pInitParams->fo.uAcTimeslotSize[nNumBitsEnabled] * BM9600_ARB_CLOCK_PERIOD_IN_NS;
        ASSERT(uTimeslotSizeNs);
    }

    rv = hwBm9600InitStep1(pInitParams);
    if (rv) {
	return rv;
    }

    if ((rv = hwBm9600InitStep2(pInitParams)) != HW_BM9600_STATUS_OK) {
	return rv;
    }

    if( pInitParams->bCleanUp ) {
      CleanUp(pInitParams);
    }
    return HW_BM9600_STATUS_OK;
}



/*
 * GetDefaultBmInitParams - Fill InitParams structure with default values
 */
void GetDefaultBmInitParams(int unit, bm9600InitParams_t *pInitParams) {

    uint32 uIndex = 0;
    int nSi = 0;
    int nAi = 0;
    int nIna = 0;
    uint32 uLink;
    uint32 uLinkEnable = 0;
    const uint32 DEFAULT_SI_JITTER_TOLERANCE = 16;
    const int SIX_NULL_GRANTS              = 6;
    const int DEFAULT_NMFULLSHIFTOFFSET      = 0x00000048;
    uint32 uCustom1Links = 0;

    sal_memset(pInitParams, 0x0, sizeof(bm9600InitParams_t));

    pInitParams->bCleanUp        = FALSE;
    pInitParams->bAllowBroadcast = 1;

    pInitParams->uNumEsetEntries = SOC_SBX_CFG(unit)->num_internal_ds_ids/16;
    if (pInitParams->uNumEsetEntries < 32) {
	pInitParams->uNumEsetEntries = 32;
    }
    pInitParams->uTimeslotSizeNs = BM9600_DEFAULT_TIMESLOT_SIZE;
    pInitParams->bInaSysportMap = 0;
    pInitParams->uSpMode = SOC_SBX_CFG(unit)->sp_mode;

    for ( nSi = 0; nSi < BM9600_NUMBER_OF_SI; nSi++ ) {
	pInitParams->si[nSi].bBringUp       = TRUE;
	pInitParams->si[nSi].uJitTolerance  = DEFAULT_SI_JITTER_TOLERANCE;
	pInitParams->si[nSi].bEvenChannelOn = TRUE;
	pInitParams->si[nSi].eChannelType   = CONTROL;
	pInitParams->si[nSi].bIsEnabled     = FALSE; /*  DeviceManager must overwrite this if needed */
	pInitParams->si[nSi].bOddChannelOn  = FALSE; /*  DeviceManager must overwrite this if needed */
	pInitParams->si[nSi].eNodeType      =  QE2000; /*  DeviceManager must overwrite this if needed */
    }

    for ( nAi = 0; nAi < BM9600_NUMBER_OF_AI_INA; nAi++ ) {
	pInitParams->ai[nAi].bBringUp   = TRUE;
	pInitParams->ai[nAi].bIsEnabled = FALSE; /*  DeviceManager must overwrite this if needed */
	pInitParams->ai[nAi].bExpandedQe2kEsetSpace = FALSE;
    }

    
    for ( nIna = 0; nIna < BM9600_NUMBER_OF_AI_INA; nIna++ ) {
	pInitParams->ina[nIna].bBringUp           = TRUE;
	pInitParams->ina[nIna].bIsEnabled         = FALSE;
	pInitParams->ina[nIna].bCritUpdNcudEnable = TRUE;
	pInitParams->ina[nIna].bRandGenEnable     = TRUE;
	pInitParams->ina[nIna].uEsetLimit         = 7;     /*  limiting to 128 for now 0x3F; */
	pInitParams->ina[nIna].bFilterPriUpdates  = FALSE;
        if (pInitParams->uSpMode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
	    pInitParams->ina[nIna].uHoldPri           = 0xD;
        }
        else {
	    pInitParams->ina[nIna].uHoldPri           = PL_INA_DEFAULT_HOLD_PRI;
        }
	pInitParams->ina[nIna].uMaxPriPri         = PL_INA_DEFAULT_MAX_PRI;

	/* INA_CONFIG.STROBE_DELAY = 10  */
	/* pInitParams->ina[nIna].uStrobeDelay  = 10; */
	pInitParams->ina[nIna].uStrobeDelay  = 8;
    }

    
    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
        (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE)) {
        pInitParams->fo.bBringUp             = TRUE;
    }
    else {
        pInitParams->fo.bBringUp             = FALSE;
    }

    /*  Get the number of logical crossbars from the system properties */
    /*  and convert into a mask to form the FO link enable mask */
    pInitParams->uNumLogicalXbars = 32;    /* MCM check this */
    for( uLink = 0; uLink < pInitParams->uNumLogicalXbars; uLink++ ) {
        uLinkEnable |= (1 << uLink);
    }
    pInitParams->fo.uLinkEnable        = uLinkEnable;
    pInitParams->fo.uDefaultBmId         = 0;
    pInitParams->fo.uForceNullGrant      = 0;
    pInitParams->fo.uNumNodes            = 0;

    /* no global link enable for pure sirius mode */
    pInitParams->fo.uUseGlobalLinkEnable = 0;

    pInitParams->fo.uMaxDisLinks         = 0;
    pInitParams->fo.uLinkDisTimerEnable  = 0;
    pInitParams->fo.uAutoLinkDisEnable   = 0;
    pInitParams->fo.uFailoverTimerEnable = 0;
    pInitParams->fo.uAutoFailoverEnable  = 0;
    pInitParams->fo.uLocalBmId           = 0;
    pInitParams->fo.uNumNullGrants       = SIX_NULL_GRANTS;

    pInitParams->fo.uFailoverTimeoutPeriod = 0;

    pInitParams->fo.uAcDegTimeslotSize = pInitParams->uTimeslotSizeNs /  BM9600_BW_CLOCK_PERIOD_200MHZ;

    /*  FO_CONFIG18.AC_TS_TO_GRANT_OFFSET = timeslot-7  */

    pInitParams->fo.uAcTsToGrantOffset = (  SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS / BM9600_BW_CLOCK_PERIOD_200MHZ ) - 7;
    pInitParams->fo.uAcTsToNmTsOffset  = 17;

   /*
    * for now, setting all timeslot sizes to same value - would want to optimize this per link up */
    for (uIndex = 0; uIndex < BM9600_NUM_TIMESLOTSIZE; ++uIndex) {
	int uTimeslotSizeNs=soc_sbx_fabric_get_timeslot_size(unit, uIndex+1, 0, SOC_SBX_CFG(unit)->bHybridMode);
	pInitParams->fo.uAcTimeslotSize[uIndex]  = uTimeslotSizeNs /  BM9600_BW_CLOCK_PERIOD_200MHZ;
        /* ensure TS_TO_GRANT_OFFSET is 7 less than smallest timeslice */
        if (pInitParams->fo.uAcTsToGrantOffset + 7 > pInitParams->fo.uAcTimeslotSize[uIndex]) {
            pInitParams->fo.uAcTsToGrantOffset = pInitParams->fo.uAcTimeslotSize[uIndex] - 7;
        }
    }

   if (pInitParams->fo.uAcTsToGrantOffset < BM9600_MIN_AC_TS_TO_GRANT_OFFSET_IN_CLOCKS_DEFAULT) {
       BM9600_ERR2("Note: Arbitration time insufficient for max esets/nodes ac_ts_grant_offset=(%d)cks dflt(%d)\n",
                   pInitParams->fo.uAcTsToGrantOffset, BM9600_MIN_AC_TS_TO_GRANT_OFFSET_IN_CLOCKS_DEFAULT);
   }


    for (uIndex = 0; uIndex < BM9600_FO_NUM_FORCE_LINK_SELECT; ++uIndex) {
	pInitParams->fo.uForceLinkSelect[uIndex] = 0;
    }

    uCustom1Links = 0;
    if (SOC_SBX_CFG(unit)->module_custom1_in_system == TRUE) {
        uCustom1Links = SOC_SBX_CFG(unit)->module_custom1_links;
    }

    for (uIndex = 0; uIndex < BM9600_FO_NUM_FORCE_LINK_ACTIVE; ++uIndex) {
	if (uIndex == SB_FAB_DEVICE_BM9600_NODE_TYPE1) {
	    /* Reserve type 1 for qe2k, matching node type in sirius, force link 21:18 to be up */
	    pInitParams->fo.uForceLinkActive[uIndex] = (0x3C0000 | uCustom1Links);
	}
	else if (uIndex == SB_FAB_DEVICE_BM9600_NODE_TYPE3) {
	    pInitParams->fo.uForceLinkActive[uIndex] = ~(uCustom1Links);
	}
        else {
	    pInitParams->fo.uForceLinkActive[uIndex] = (0 | uCustom1Links);
	}
    }

    
    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
        (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE)) {
        pInitParams->bw.bBringUp = TRUE;
    }
    else {
        pInitParams->bw.bBringUp = FALSE;
    }

    pInitParams->bw.uMaxResponseLatencyInTimeSlots = 0;
    pInitParams->bw.uRxSotToBaaOffset          = 65;   /* Derived experimentally - may vary by timeslot size */
    pInitParams->bw.bTagCheckEnable            = TRUE;
    pInitParams->bw.uActiveBagNum              = 0;
    pInitParams->bw.uActiveVoqNum              = 0;
    pInitParams->bw.bBandwidthAllocationEnable = FALSE;
    pInitParams->bw.bWredEnable                = FALSE;
    pInitParams->bw.bAllocationScaleEnable     = FALSE;
    pInitParams->bw.bIgnoreTimeout             = FALSE;
    sal_memset(pInitParams->bw.auVoqNumInBag,        0x00, sizeof(pInitParams->bw.auVoqNumInBag) );
    sal_memset(pInitParams->bw.azCurveTable,         0x00, sizeof(pInitParams->bw.azCurveTable) );
    pInitParams->uDemandClks                    = 0; /* assumes wred disabled */
    pInitParams->bw.uSegmentSize                = 0;
    pInitParams->bw.uEpochLength                = 0;
    pInitParams->bw.uFetchSwap                  = 0;
    pInitParams->bw.uDistribSwap                = 0;


    /* For A1 and A0, we may need to store NM data even in crossbar only mode */
    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
        (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE) || 
	(SOC_PCI_REVISION(pInitParams->m_nBaseAddr) == BCM88130_A0_REV_ID) ||
	(SOC_PCI_REVISION(pInitParams->m_nBaseAddr) == BCM88130_A1_REV_ID)) {
	pInitParams->nm.bBringUp = TRUE;
	pInitParams->nm.bRandGenEnable  = TRUE;
	pInitParams->nm.bInitRankerTables  = TRUE;
    } else {
	pInitParams->nm.bBringUp = FALSE;
	pInitParams->nm.bRandGenEnable  = FALSE;
	pInitParams->nm.bInitRankerTables  = FALSE;
    }

    if (pInitParams->nm.bInitRankerTables == TRUE) {
        for (uIndex = 0; uIndex < NM_INGRESS_RANKER_TABLE_NUM_ENTRIES; uIndex++) {
            pInitParams->nm.pIngressRankerTable[uIndex] = bm9600IngressRankerTable[uIndex];
        }
        for (uIndex = 0; uIndex < NM_EGRESS_RANKER_TABLE_NUM_ENTRIES; uIndex++) {
            pInitParams->nm.pEgressRankerTable[uIndex] = bm9600EgressRankerTable[uIndex];
        }
        for (uIndex = 0; uIndex < NM_RAND_NUM_TABLE_NUM_ENTRIES; uIndex++) {
            pInitParams->nm.pRandTable[uIndex] = bm9600RandTable[uIndex];
        }
    }

    pInitParams->nm.bEnableAllEgress = TRUE;

    /* NM_CONFIG.GRANT_TO_FULL_SHIFT_OFFSET = 8  */
    /* NM_CONFIG.TS_TO_ESET_SHIFT_OFFSET = E+32 */
    pInitParams->nm.uTsToEsetShiftOffset     = pInitParams->uNumEsetEntries  + 32;
    pInitParams->nm.uNmFullShiftOffset       = DEFAULT_NMFULLSHIFTOFFSET;
    pInitParams->nm.uGrantToFullShiftOffset  = 8;/*( uTimeslotSizeNs / BM9600_BW_CLOCK_PERIOD_200MHZ ) - 20;8; */
    pInitParams->nm.uNmDualGrantConfig0      = 0;
    pInitParams->nm.uNmDualGrantConfig1      = 0;
    pInitParams->nm.uNmDualGrantConfig2      = 0;


    /* By default, set to 3 errors in 255...will be overwritten later */
    pInitParams->uSiLsWindow = 255;
    pInitParams->uSiLsThreshold = 3;

    /*
     * Additional setup for crossbar, should it be enabled
     */

    pInitParams->xb.uTsJitterTolerance    = 22;
    pInitParams->xb.eXcfgMode     = XCFG_SE_MODE;
    pInitParams->xb.uInitMode     = 1;
    pInitParams->xb.bEnableSotPolicing = TRUE;

    pInitParams->pi.bBringUp = TRUE;
    pInitParams->xb.bBringUp = TRUE;
}

/*
 * GetDefaultSeInitParams - fill InitParams struct with defaults for Se block
 */
void GetDefaultSeInitParams(int unit, bm9600InitParams_t *pInitParams) {

    int nSi = 0;
    int nAi = 0;
    int nIna = 0;
    const uint32 DEFAULT_SI_JITTER_TOLERANCE = 16;
    const uint32 XB_IDENTITY_INIT_MODE = 1;
    const uint32 XB_DEFAULT_JITTER_TOLERANCE = 22;

    sal_memset(pInitParams, 0x0, sizeof(bm9600InitParams_t));

    pInitParams->bCleanUp = FALSE;

    for ( nSi = 0; nSi < BM9600_NUMBER_OF_SI; nSi++ ) {
	pInitParams->si[nSi].bBringUp      = TRUE;
	pInitParams->si[nSi].uJitTolerance = DEFAULT_SI_JITTER_TOLERANCE;
	pInitParams->si[nSi].bEvenChannelOn = TRUE;
	pInitParams->si[nSi].eChannelType = DATA;
    }

    /*  Default Clock period for Serializers */
    for ( nSi = 0; nSi < BM9600_NUMBER_OF_SI; nSi++ ) {
	pInitParams->serx[nSi].uPeriod = BM9600_KA_SERX_CLK_PERIOD_PS;
    }

    for ( nAi = 0; nAi < BM9600_NUMBER_OF_AI_INA; nAi++ ) {
	pInitParams->ai[nAi].bBringUp   = FALSE;
	pInitParams->ai[nAi].bIsEnabled = FALSE;
    }

    for ( nIna = 0; nIna < BM9600_NUMBER_OF_AI_INA; nIna++ ) {
	pInitParams->ina[nIna].bBringUp   = FALSE;
	pInitParams->ina[nIna].bIsEnabled = FALSE;
    }

    pInitParams->fo.bBringUp    = FALSE;
    pInitParams->fo.uLinkEnable = 0;
    pInitParams->bw.bBringUp    = FALSE;
    pInitParams->nm.bBringUp    = FALSE;
    pInitParams->pi.bBringUp    = TRUE;

    pInitParams->xb.bBringUp      = TRUE;
    pInitParams->xb.uInitMode     = XB_IDENTITY_INIT_MODE;
    pInitParams->xb.eXcfgMode     = XCFG_SE_MODE;
    pInitParams->xb.bEnableSotPolicing = TRUE;
    pInitParams->xb.bBypass       = FALSE;
    pInitParams->xb.uTsJitterTolerance    = XB_DEFAULT_JITTER_TOLERANCE;
    pInitParams->xb.bCheckGlobalAlignment = TRUE;
    pInitParams->xb.uPassThru = 3;
}

/*
 *  hwBm9600GetTimeSlotSizeInClocks -  calculate timeslot as it is variable
 */
uint32 hwBm9600GetTimeSlotSizeInClocks(const bm9600InitParams_t *pInitParams){
  /*  uint32 uTimeSlotInClocks = (2 * uNodeNum) + BM9600_NM_TIMESLOT_OVERHEAD_IN_CLOCKS; */
  return ( pInitParams->uTimeslotSizeNs / BM9600_ARB_CLOCK_PERIOD_IN_NS );
}

/*
 * GetIncrementalRegMapBaseAddr
 */
uint32 GetIncrementalRegMapBaseAddr(void) {
  static uint32 uNumDevices = 0;
  return BM9600_DEFAULT_BASE_ADDRESS + (uNumDevices++ * BM9600_DEFAULT_REGMAP_OFFSET);
}

#if 0

-------  INTERRUPTS NOT IMPLEMENTED YET ------------------



uint32 CheckInterruptReg(int nOffset, uint32 uValue, BOOL bCheck, BOOL bClear, BOOL bReturnMasked /* TRUE */ ){

  uint32 uData = SAND_HAL_READ_ADDR(pInitParams->m_nBaseAddr | nOffset);
  uint32 uMask = SAND_HAL_READ_ADDR(pInitParams->m_nBaseAddr | (nOffset+4));
  uint32 uReturnValue;
  CZString sName = m_RegSet.TranslateAddress(nOffset);

  if( bCheck && ( (uData & ~uMask) != uValue) ) {
    BM9600_ERR( "%s data=0x%x (mask=0x%x) does not match expected=0x%x. Fields below:\n       %s\n",
               _T sName, uData, uMask, uValue,
               _T ZFoldString(m_RegSet.TranslateInterruptData(nOffset, uData, uMask, REGSET_SET_FIELDS), 25, "         "));
  } else{
    if( uData ) {
      BM9600_LOG3( "  %s data=0x%x (mask=0x%x) \n", _T sName, uData, uMask);
      BM9600_LOG3( "    fields: %s\n",
                _T ZFoldString(m_RegSet.TranslateInterruptData(nOffset, uData, uMask, REGSET_SET_FIELDS), 25, "           "));
    }
  }

  if ( bClear && uData ) {
    /* need to turn off backdoor to avoid RAW hazard */
    BOOL bAcc = gZM.GetAttribute("acc", 0);
    gZM.SetAttribute("acc=0");
    SAND_HAL_WRITE_ADDR(pInitParams->m_nBaseAddr | nOffset, uData);
    uReturnValue = SAND_HAL_READ_ADDR(pInitParams->m_nBaseAddr | nOffset);
    gZM.SetAttribute("acc", bAcc);
    /*  if the interrupt is masked, it may be getting triggered continuously */
    if( (uReturnValue & ~uMask) != 0x0 ) {
      BM9600_ERR2("%s CANNOT clear (return value: 0x%08x)\n",_T sName, uReturnValue);
    } else {
      BM9600_LOG1( "  %s cleared\n", _T sName);
    }
  }
  if( bReturnMasked ) {
    uData = uData & ~uMask;
  }
  return uData;
}

/*  Clear any and all interrupts set and flush all history of */
/*  any interrupt transitions */
void ClearAllInterrupts() {
  uint32 uBlock;
  for ( uBlock = 0; uBlock < BM9600_NUMBER_OF_AI_INA; uBlock++ ) {
    hwBm9600TraceErrorRegs("ai", TRUE, FALSE);
    hwBm9600TraceErrorRegs("ina", TRUE, FALSE);
  }
  for ( uBlock = 0; uBlock < BM9600_NUMBER_OF_SI; uBlock++ ) {
    hwBm9600TraceErrorRegs("si", TRUE, FALSE);
  }

  hwBm9600TraceErrorRegs("fo", TRUE, FALSE);
  hwBm9600TraceErrorRegs("bw", TRUE, FALSE);
  hwBm9600TraceErrorRegs("nm", TRUE, FALSE);
  hwBm9600TraceErrorRegs("pi", TRUE, FALSE);
  hwBm9600TraceErrorRegs("xb", TRUE, FALSE);

  CZMPInterrupt* pInt = (CZMPInterrupt*)GetModule("dut_int");
  ASSERT(pInt);
  pInt->FlushTransitions();

}

void hwBm9600TraceErrorRegs(char *sBlockName, BOOL bClear, BOOL bUnMask ) {
  BM9600_LOG3( " called for block %s, Clear:%d, UnMask:%d\n", _T sBlockName, bClear, bUnMask);
  CZRegSet zPartialRegSet;
  PopulateRegsPl(&zPartialRegSet, sBlockName);
  {
    /*  find any register that is an interrupt register */
    CZIter iter(zPartialRegSet.m_regMap);
    CZRegItem *pRegItem;
    uint32 uAddress;
    uint32 uRegData;

    while(iter.GetNext((void*&)uAddress, (void*&)pRegItem)) {
      ASSERT(pRegItem);

      if ( pRegItem->m_bInterrupt ) {
        uRegData = SAND_HAL_READ_OFFS_RAW(pInitParams->m_nBaseAddr, uAddress);
        /*  Only trace non-zero registers */
        if( uRegData != 0 ) {
          CZString sReg = zPartialRegSet.TranslateData(uAddress, uRegData, REGSET_SET_FIELDS);
          BM9600_LOG2("Trace non-zero interrupt register %s (%s)\n",
                    _T zPartialRegSet.TranslateAddress(uAddress), _T sReg);
          if( bClear ) {
            /*  Read again since any read to clear's will */
            /*  now have been cleared */
            uRegData = SAND_HAL_READ_OFFS_RAW(pInitParams->m_nBaseAddr, uAddress);
            /*  And write back to clear any W1TC fields */
            SAND_HAL_WRITE_OFFS_RAW(pInitParams->m_nBaseAddr, uAddress, uRegData);
          }
        }
        /* Mask are auto place to next address above the register */
        if (bUnMask){
          uRegData = SAND_HAL_READ_OFFS_RAW(pInitParams->m_nBaseAddr, uAddress+4);
          if( uRegData != 0 ) { /* If mask isn't cleared, clear it */
            CZString sReg = zPartialRegSet.TranslateData(uAddress+4, uRegData, REGSET_SET_FIELDS);
            BM9600_LOG1("Trace clear interrupt mask register %s (%s)\n",
                       _T zPartialRegSet.TranslateAddress(uAddress+4), _T sReg);
            SAND_HAL_WRITE_OFFS_RAW(pInitParams->m_nBaseAddr, uAddress+4, 0x0);
          }
        }
      }
    }
  }
}


void TraceInterruptReg(int nOffset, uint32 uValue, BOOL bClear) {
  CZString sTop = m_RegSet.TranslateData(nOffset, uValue, REGSET_SET_FIELDS);
  UINT32 uInterruptReg;

  BM9600_LOG2( "Trace of register 0x%08x=0x%08x started\n", nOffset, uValue);
  BM9600_LOG1( "%s", _T sTop);

  if( nOffset == SAND_HAL_REG_OFFSET(PL, PI_INTERRUPT) ) {
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_INTERRUPT_UNIT_INTERRUPT_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        uint32 uOffset =  SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT0) + (8 * uBitLocal);
        uint32 uNewData = SAND_HAL_READ_ADDR(pInitParams->m_nBaseAddr | uOffset);
        TraceInterruptReg(uOffset, uNewData, TRUE);
      }
    }

    if( bClear ) {
      uInterruptReg = SAND_HAL_READ(pInitParams->m_nBaseAddr, PL, PI_INTERRUPT);
      SAND_HAL_WRITE( pInitParams->m_nBaseAddr, PL, PI_INTERRUPT, uInterruptReg );
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT0) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT0_INA_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("ina", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT1) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT1_INA_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("ina", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT2) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT2_INA_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("ina", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT3) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT3_AI_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("ai", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT4) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT4_AI_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("ai", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT5) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT5_AI_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("ai", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT6) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT6_SI_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("si", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT7) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT7_SI_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("si", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT8) ){
    for (uint32 uBitLocal=0; uBitLocal > (SAND_HAL_PL_PI_UNIT_INTERRUPT8_SI_MSB + 1); ++uBitLocal) {
      uint32 uRegBit =  0x1 & (uValue >> uBitLocal);
      if (uRegBit == 1) {
        hwBm9600TraceErrorRegs("si", TRUE);
      }
    }
  } else if (nOffset == SAND_HAL_REG_OFFSET(PL, PI_UNIT_INTERRUPT9) ){
    if( SAND_HAL_GET_FIELD(PL,PI_UNIT_INTERRUPT9, XB, uValue) == 1 ) {
      hwBm9600TraceErrorRegs("xb", bClear);
    }
    if( SAND_HAL_GET_FIELD(PL,PI_UNIT_INTERRUPT9, NM, uValue) == 1 ) {
      hwBm9600TraceErrorRegs("nm", bClear);
    }
    if( SAND_HAL_GET_FIELD(PL,PI_UNIT_INTERRUPT9, FO, uValue) == 1 ) {
      hwBm9600TraceErrorRegs("fo", bClear);
    }
    if( SAND_HAL_GET_FIELD(PL,PI_UNIT_INTERRUPT9, BW, uValue) == 1 ) {
      hwBm9600TraceErrorRegs("bw", bClear);
    }
  } else {
    ZSIM_UPGRADE( "Don't know how to trace and clear non PL_INTERRUPT reg.");
  }
}
#endif

/*
 *  hwBm9600WriteFoAcTimeslotSize
 */
void hwBm9600WriteFoAcTimeslotSize(const bm9600InitParams_t *pInitParams,  uint32 nIndex, uint32 nTsLength) {
    uint32 uVal;

    if (nTsLength > 0x7ff) {
        nTsLength = 0x7ff; /* field is only 11bits long */
    }
    ASSERT((SAND_HAL_PL_FO_CONFIG19_OFFSET+15*4) == SAND_HAL_PL_FO_CONFIG34_OFFSET);
    uVal = SAND_HAL_READ_INDEX(pInitParams->m_nBaseAddr, PL, FO_CONFIG19, nIndex / 2);
    if (nIndex%2)
	uVal = SAND_HAL_MOD_FIELD(PL, FO_CONFIG19, AC_TIMESLOT_SIZE1, uVal, nTsLength);
    else
	uVal = SAND_HAL_MOD_FIELD(PL, FO_CONFIG19, AC_TIMESLOT_SIZE0, uVal, nTsLength);

    SAND_HAL_WRITE_INDEX(pInitParams->m_nBaseAddr, PL, FO_CONFIG19, nIndex / 2, uVal);
}

/*
 * ReadFoAcTimeslotSize
 */
uint32 ReadFoAcTimeslotSize(bm9600InitParams_t *pInitParams,  uint32 nIndex) {
    uint32 uVal;

    ASSERT((SAND_HAL_PL_FO_CONFIG19_OFFSET+15*4) == SAND_HAL_PL_FO_CONFIG34_OFFSET);

    uVal = SAND_HAL_READ_INDEX(pInitParams->m_nBaseAddr, PL, FO_CONFIG19, nIndex / 2);
    if (nIndex%2)
	return SAND_HAL_GET_FIELD(PL, FO_CONFIG19, AC_TIMESLOT_SIZE1, uVal);
    else
	return SAND_HAL_GET_FIELD(PL, FO_CONFIG19, AC_TIMESLOT_SIZE0, uVal);
}

# endif /* BCM_BM9600_SUPPORT */
