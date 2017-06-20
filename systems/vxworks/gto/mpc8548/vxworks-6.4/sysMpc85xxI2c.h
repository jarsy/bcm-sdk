/* sysMpc85xxI2c.h - Mpc85xx I2C Driver Header Module */

/* $Id: sysMpc85xxI2c.h,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,21feb05,gtf  created.
*/

/*
DESCRIPTION

I2C Driver Header (Low Level Routines) Module
Mpc85xx Memory Controller (MPC85XX - PowerPlus Architecture)

Notes:
	1. The low level routines were modeled after the original
	   driver written by Pamela Wolfe.
*/

#ifndef	INCsysMpc85xxI2ch
#define	INCsysMpc85xxI2ch

#include <vxWorks.h>
#include <config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mpc85xx Register Addresses */
#define MPC85XX_I2C_ADR_REG		       	(0x00000)
#define MPC85XX_I2C_FREQ_DIV_REG		(0x00004)
#define MPC85XX_I2C_CONTROL_REG			(0x00008)
#define MPC85XX_I2C_STATUS_REG			(0x0000c)
#define MPC85XX_I2C_DATA_REG			(0x00010)
#define MPC85XX_I2C_DIG_FILTER_REG		(0x00014)

/* Mpc85xx Register masks */

#define MPC85XX_I2C_ADDRESS_REG_MASK 	0xFE
#define MPC85XX_I2C_FREQ_DIV_REG_MASK	0x3F
#define MPC85XX_I2C_CONTROL_REG_MASK	0xFD
#define MPC85XX_I2C_STATUS_REG_MASK		0xFF
#define MPC85XX_I2C_DATA_REG_MASK		0xFF
#define MPC85XX_I2C_DIG_FILTER_REG_MASK	0x3F


/* Mpc85xx Control register values */

#define MPC85XX_I2C_CONTROL_REG_MEN	    0x80	/* module enable */
#define MPC85XX_I2C_CONTROL_REG_MIEN	0x40  	/* module interrupt enable */
#define MPC85XX_I2C_CONTROL_REG_MSTA	0x20  	/* master/slave mode */
#define MPC85XX_I2C_CONTROL_REG_MTX	    0x10  	/* transmit/receiver mode  */
#define MPC85XX_I2C_CONTROL_REG_TXAK	0x08  	/* transfer ack enable */
#define MPC85XX_I2C_CONTROL_REG_RSTA	0x04  	/* repeat start */
#define MPC85XX_I2C_CONTROL_REG_BCST	0x01  	/* accept broadcast messages */


/* Mpc85xx Status register values */

#define MPC85XX_I2C_STATUS_REG_MCF      0x80  	/* data transferring */
#define MPC85XX_I2C_STATUS_REG_MAAS	    0x40  	/* addressed as a slave */
#define MPC85XX_I2C_STATUS_REG_MBB	    0x20  	/* bus busy */
#define MPC85XX_I2C_STATUS_REG_MAL	    0x10  	/* arbitration lost */
#define MPC85XX_I2C_STATUS_REG_BCSTM    0x08  	/* broadcast match */
#define MPC85XX_I2C_STATUS_REG_SRW	    0x04  	/* slave read/write */
#define MPC85XX_I2C_STATUS_REG_MIF	    0x02  	/* module interrupt */
#define MPC85XX_I2C_STATUS_REG_RXAK	    0x01  	/* receive ack */


IMPORT int i2cCycleMpc85xxWrite (int, unsigned char);
IMPORT void i2cCycleMpc85xxDelay (int);
IMPORT int i2cCycleMpc85xxStart (int);
IMPORT int i2cCycleMpc85xxStop (int);
IMPORT int i2cCycleMpc85xxRead (int,unsigned char *,int);
IMPORT int i2cCycleMpc85xxAckIn (int);
IMPORT int i2cCycleMpc85xxAckOut (int);
IMPORT int i2cCycleMpc85xxKnownState (int);


#ifdef __cplusplus
}
#endif


#endif /* INCsysMpc85xxI2ch */
