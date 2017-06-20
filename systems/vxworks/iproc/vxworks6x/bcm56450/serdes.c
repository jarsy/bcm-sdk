/* serdes.c - serdes access functions */

/*
 * Copyright (c) 2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,07oct13,dnb  written from u-boot reference code
*/

#include <vxWorks.h>
#include <vxBusLib.h>
#include <hwif/vxbus/vxBus.h>
#include "config.h"

#define IPROC_WRAP_MISC_CONTROL 		0x1803fc24

#define IPROC_WRAP_MISC_CONTROL__UNICORE_SERDES_MDIO_SEL 2
#define IPROC_WRAP_MISC_CONTROL__UNICORE_SERDES_CTRL_SEL 1
#define IPROC_WRAP_MISC_CONTROL__IPROC_MDIO_SEL 3

#define PHY_REG_BLK_ADDR			0x1f

#define XGXS16G_2p5G_ID(id2) ((id2 & 0xff) == 0xf)

#define XGXS16G_IEEE0BLK_IEEECONTROL0r		0x00000000
#define XGXS16G_SERDESID_SERDESID0r		0x00008310
#define IEEE0BLK_IEEECONTROL0_RST_HW_MASK	0x8000

#define XGXSBLK0_XGXSCONTROL_MODE_10G_IndLane   6
#define XGXSBLK0_XGXSCONTROL_MODE_10G_SHIFT     8
#define XGXSBLK0_XGXSCONTROL_HSTL_MASK          0x0020
#define XGXSBLK0_XGXSCONTROL_CDET_EN_MASK       0x0008
#define XGXSBLK0_XGXSCONTROL_EDEN_MASK          0x0004
#define XGXSBLK0_XGXSCONTROL_AFRST_EN_MASK      0x0002
#define XGXSBLK0_XGXSCONTROL_TXCKO_DIV_MASK     0x0001

#define XGXS16G_XGXSBLK0_XGXSCONTROLr	        		0x00008000
#define XGXS16G_XGXSBLK0_MISCCONTROL1r				0x0000800e

#define XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_AUTODET_MASK  	0x0002
#define XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_VAL_MASK      	0x0001

#define XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK       	0x2000
#define XGXS16G_XGXSBLK0_XGXSSTATUSr				0x00008001
#define XGXSBLK0_XGXSSTATUS_TXPLL_LOCK_MASK             	0x0800

#define XGXS16G_XGXSBLK7_EEECONTROLr				0x00008150
#define XGXS16G_REMOTEPHY_MISC5r				0x0000833e
#define SERDESDIGITAL_CONTROL1000X3_FIFO_ELASICITY_TX_RX_MASK   0x0006
#define XGXS16G_SERDESDIGITAL_CONTROL1000X1r			0x00008300
#define XGXS16G_SERDESDIGITAL_CONTROL1000X2r			0x00008301
#define XGXS16G_SERDESDIGITAL_CONTROL1000X3r			0x00008302
#define XGXS16G_COMBO_IEEE0_MIICNTLr				0x0000ffe0
#define XGXS16G_COMBO_IEEE0_AUTONEGADVr				0x0000ffe4
#define SERDESDIGITAL_CONTROL1000X1_CRC_CHECKER_DISABLE_MASK    0x0080
#define SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK     0x0040
#define XGXS16G_BAM_NEXTPAGE_MP5_NEXTPAGECTRLr			0x00008350
#define XGXSBLK1_LANECTRL0_CL36_PCS_EN_RX_MASK                  0x00f0
#define XGXSBLK1_LANECTRL0_CL36_PCS_EN_TX_MASK                  0x000f
#define XGXS16G_AN73_PDET_PARDET10GCONTROLr			0x00008131
#define XGXS16G_XGXSBLK1_LANECTRL0r				0x00008015
#define CL73_USERB0_CL73_BAMCTRL1_CL73_BAMEN_MASK               0x8000
#define XGXS16G_SERDESDIGITAL_CONTROL1000X1r			0x00008300
#define XGXS16G_BAM_NEXTPAGE_MP5_NEXTPAGECTRLr			0x00008350

/* MII Control Register: bit definitions */

#define MII_CTRL_FS_2500        (1 << 5) /* Force speed to 2500 Mbps */
#define MII_CTRL_SS_MSB         (1 << 6) /* Speed select, MSb */
#define MII_CTRL_CST            (1 << 7) /* Collision Signal test */
#define MII_CTRL_FD             (1 << 8) /* Full Duplex */
#define MII_CTRL_RAN            (1 << 9) /* Restart Autonegotiation */
#define MII_CTRL_IP             (1 << 10) /* Isolate Phy */
#define MII_CTRL_PD             (1 << 11) /* Power Down */
#define MII_CTRL_AE             (1 << 12) /* Autonegotiation enable */
#define MII_CTRL_SS_LSB         (1 << 13) /* Speed select, LSb */
#define MII_CTRL_LE             (1 << 14) /* Loopback enable */
#define MII_CTRL_RESET          (1 << 15) /* PHY reset */

#define MII_CTRL_SS(_x)         ((_x) & (MII_CTRL_SS_LSB|MII_CTRL_SS_MSB))
#define MII_CTRL_SS_10          0
#define MII_CTRL_SS_100         (MII_CTRL_SS_LSB)
#define MII_CTRL_SS_1000        (MII_CTRL_SS_MSB)
#define MII_CTRL_SS_INVALID     (MII_CTRL_SS_LSB | MII_CTRL_SS_MSB)
#define MII_CTRL_SS_MASK        (MII_CTRL_SS_LSB | MII_CTRL_SS_MSB)

/* Refer to iproc/bcmdrivers/gmac/src/et/sys/etc.c */
#define CONFIG_IPROC_SDK_MGT_PORT_HANDOFF (1)

void sysUsDelay(INT32 count);

METHOD_DECL(phyRead);
METHOD_DECL(phyWrite);

/*******************************************************************************
*
* serdesBlkSet
*
*
* RETURNS: none
*
* ERRNO: N/A
*/

void serdesBlkSet
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr,
    UINT16  blk
    )
    {
    UINT16 blkAddr;

    FUNCPTR  phyRead = vxbDevMethodGet(devId, (UINT32)&phyRead_desc);
    FUNCPTR  phyWrite = vxbDevMethodGet(devId, (UINT32)&phyWrite_desc);

    if(phyRead)
	phyRead(devId, 0, phyAddr, PHY_REG_BLK_ADDR, &blkAddr);

    if(phyWrite && (blkAddr != blk))
	phyWrite(devId, 0, phyAddr, PHY_REG_BLK_ADDR, blk);

    return;
    }

/*******************************************************************************
*
* serdesRegWrite 
*
*
* RETURNS: none
*
* ERRNO: N/A
*/

void serdesRegWrite
    (
    VXB_DEVICE_ID 	devId,
    UINT8		phyAddr,
    UINT16 		reg,
    UINT16  		data
    )
    {
    UINT16 blk = reg & 0x7ff0;
    UINT8  off = reg & 0x0f;

    FUNCPTR  phyWrite = vxbDevMethodGet(devId, (UINT32)&phyWrite_desc);

    serdesBlkSet(devId, phyAddr, blk);

    if(phyWrite)
	phyWrite(devId, 0, phyAddr, off, data);

    return;
    }

/*******************************************************************************
*
* serdesRegRead
*
*
* RETURNS: none
*
* ERRNO: N/A
*/

UINT16 serdesRegRead
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr,
    UINT16 reg
    )
    {
    UINT16 blk = reg & 0x7ff0;
    UINT8  off = reg & 0x0f;
    UINT16 data;

    FUNCPTR  phyRead = vxbDevMethodGet(devId, (UINT32)&phyRead_desc);

    serdesBlkSet(devId, phyAddr, blk);

    if(phyRead)
	phyRead(devId, 0, phyAddr, off, &data);

    return data;
    }

/*******************************************************************************
*
* serdesIdGet
*
*
* RETURNS: none
*
* ERRNO: N/A
*/

UINT16 serdesIdGet
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr,
    UINT16 off
    )
    {
    return serdesRegRead(devId, phyAddr, off + XGXS16G_SERDESID_SERDESID0r);
    }

/*******************************************************************************
*
* serdesReset
*
*
* RETURNS: OK if reset is complete
*
* ERRNO: N/A
*/

STATUS serdesReset
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr
    )
    {
    STATUS ret = ERROR;
    int    i;

    UINT16 ctrl = serdesRegRead(devId, phyAddr,XGXS16G_IEEE0BLK_IEEECONTROL0r);
    ctrl |= IEEE0BLK_IEEECONTROL0_RST_HW_MASK;
    serdesRegWrite(devId, phyAddr, XGXS16G_IEEE0BLK_IEEECONTROL0r, ctrl);

    for(i = 0 ; i < 1000; i++)
	{
	if((serdesRegRead(devId, phyAddr, XGXS16G_IEEE0BLK_IEEECONTROL0r) &
           IEEE0BLK_IEEECONTROL0_RST_HW_MASK) == 0)
	    {
            ret = OK;
            break;
	    }
	}
    return ret;
    }

/*******************************************************************************
*
* serdesCoreReset
*
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void serdesCoreReset
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr
    )
    {
    UINT16	serdesId2;
    UINT16      data16;

    serdesId2 = serdesIdGet(devId, phyAddr, 2);

    /* unlock lane */
    data16 = serdesRegRead(devId, phyAddr, 0x833c);
    data16 &= ~(0x0040);
    serdesRegWrite(devId, phyAddr, 0x833c, data16);

    /* reset the core */
    if ( phyAddr == 1 ) 
	{
	/* stop pll sequencer and configure the core into correct mode */
	data16 = (XGXSBLK0_XGXSCONTROL_MODE_10G_IndLane <<
		  XGXSBLK0_XGXSCONTROL_MODE_10G_SHIFT) |
		  XGXSBLK0_XGXSCONTROL_HSTL_MASK |
		  XGXSBLK0_XGXSCONTROL_CDET_EN_MASK |
		  XGXSBLK0_XGXSCONTROL_EDEN_MASK |
		  XGXSBLK0_XGXSCONTROL_AFRST_EN_MASK |
		  XGXSBLK0_XGXSCONTROL_TXCKO_DIV_MASK;
	serdesRegWrite(devId, phyAddr, XGXS16G_XGXSBLK0_XGXSCONTROLr, data16);

	/* Disable IEEE block select auto-detect. 
	 * The driver will select desired block as necessary.
	 * By default, the driver keeps the XAUI block in
	 * IEEE address space.
	 */
	data16 = serdesRegRead(devId, phyAddr, XGXS16G_XGXSBLK0_MISCCONTROL1r);
	if (XGXS16G_2p5G_ID(serdesId2)) 
	    {
	    data16 &= ~(XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_AUTODET_MASK |
			XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_VAL_MASK);
	    } 
	else 
	    {
	    data16 &= ~(XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_AUTODET_MASK |
			XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_VAL_MASK);
	    data16 |= XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_VAL_MASK;
	    }

	serdesRegWrite(devId, phyAddr, XGXS16G_XGXSBLK0_MISCCONTROL1r, data16);

	/* disable in-band MDIO. PHY-443 */
	data16 = serdesRegRead(devId, phyAddr, 0x8111);
	/* rx_inBandMdio_rst */
	data16 |= 1 << 3;
        serdesRegWrite(devId, phyAddr, 0x8111, data16);
	}

    return;
    }

/*******************************************************************************
*
* serdesPllStart
*
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void serdesPllStart
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr
    )
    {
    UINT16 data16;
    int	   i;

    if(phyAddr == 1)
	{
	/* Start PLL Sequencer ... */
        data16 = serdesRegRead(devId, phyAddr, XGXS16G_XGXSBLK0_XGXSCONTROLr);
	data16 |= XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK;
	serdesRegWrite(devId, phyAddr, XGXS16G_XGXSBLK0_XGXSCONTROLr, data16);

	/* wait for PLL to lock */
	for(i = 0 ; i < 250 ; i++)
	    {
            data16 = serdesRegRead(devId, phyAddr, XGXS16G_XGXSBLK0_XGXSSTATUSr);
	    if(data16 & XGXSBLK0_XGXSSTATUS_TXPLL_LOCK_MASK)
		{
		break;
		}
	    sysUsDelay(10);
	    }
	}
    }

#if (CONFIG_IPROC_SDK_MGT_PORT_HANDOFF)
void handoff ()
{
    (*(UINT32*)IPROC_WRAP_MISC_CONTROL) = 0;
}
#endif /* CONFIG_IPROC_SDK_MGT_PORT_HANDOFF */
/*******************************************************************************
*
* serdesInit
*
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void serdesInit
    (
    VXB_DEVICE_ID devId,
    UINT8  phyAddr
    )
    {
    UINT16 data16;

    /* unlock lane */
    data16 = serdesRegRead(devId, phyAddr, 0x833c);
    data16 &= ~(0x0040);
    serdesRegWrite(devId, phyAddr, 0x833c, data16);

    /* disable CL73 BAM */
    data16 = serdesRegRead(devId, phyAddr, 0x8372);
    data16 &= ~(CL73_USERB0_CL73_BAMCTRL1_CL73_BAMEN_MASK);
    serdesRegWrite(devId, phyAddr, 0x8372, data16);

    data16 = SERDESDIGITAL_CONTROL1000X1_CRC_CHECKER_DISABLE_MASK |
	     SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK;
    /*
     * Put the Serdes in SGMII mode
     * bit0 = 0; in SGMII mode
     */
    serdesRegWrite(devId, phyAddr, XGXS16G_SERDESDIGITAL_CONTROL1000X1r, data16);

    /* Initialialize 1G and fullduplex */
    data16 = MII_CTRL_FD | MII_CTRL_SS_1000;
    /* set autoneg */
    data16 |= MII_CTRL_AE | MII_CTRL_RAN;
   serdesRegWrite(devId, phyAddr, XGXS16G_COMBO_IEEE0_MIICNTLr, data16);

    /* Disable 10G parallel detect */
    data16 = 0;
    serdesRegWrite(devId, phyAddr, XGXS16G_AN73_PDET_PARDET10GCONTROLr, data16);

    /* Disable BAM mode and Teton mode */
    serdesRegWrite(devId, phyAddr, XGXS16G_BAM_NEXTPAGE_MP5_NEXTPAGECTRLr, data16);

    /* Enable lanes */
    data16 = serdesRegRead(devId, phyAddr, XGXS16G_XGXSBLK1_LANECTRL0r);
    data16 |= XGXSBLK1_LANECTRL0_CL36_PCS_EN_RX_MASK |
	XGXSBLK1_LANECTRL0_CL36_PCS_EN_TX_MASK;
    serdesRegWrite(devId, phyAddr, XGXS16G_XGXSBLK1_LANECTRL0r, data16);

    /* set elasticity fifo size to 13.5k to support 12k jumbo pkt size*/
    data16 = serdesRegRead(devId, phyAddr, XGXS16G_SERDESDIGITAL_CONTROL1000X3r);
    data16 &= SERDESDIGITAL_CONTROL1000X3_FIFO_ELASICITY_TX_RX_MASK;
    data16 |= (1 << 2);
    serdesRegWrite(devId, phyAddr, XGXS16G_SERDESDIGITAL_CONTROL1000X3r, data16);

    /* Enable LPI passthru' for native mode EEE */
    data16 = serdesRegRead(devId, phyAddr, XGXS16G_REMOTEPHY_MISC5r);
    data16 |= 0xc000;
    serdesRegWrite(devId, phyAddr,  XGXS16G_REMOTEPHY_MISC5r, data16);
    data16 = serdesRegRead(devId, phyAddr, XGXS16G_XGXSBLK7_EEECONTROLr);
    data16 |= 0x0007;
    serdesRegWrite(devId, phyAddr, XGXS16G_XGXSBLK7_EEECONTROLr, data16);


#if (CONFIG_IPROC_SDK_MGT_PORT_HANDOFF)
    {
        #include "wdLib.h"
        #define SECONDS (30) 

        WDOG_ID myWatchDogId; 
        /* Create watchdog */  

        if ((myWatchDogId = wdCreate()) == NULL) 
            return; 

        /* Set timer to go off in SECONDS - printing a message to stdout */ 
        if (wdStart (myWatchDogId, sysClkRateGet() * SECONDS, (FUNCPTR)handoff, 0) 
            == NULL)
            return;

    }
#endif /*  CONFIG_IPROC_SDK_MGT_PORT_HANDOFF */

    return;
    }
