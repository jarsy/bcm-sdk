/* 40iProcPCIe.cdf - Component configuration file */

/*
 * Copyright (c) 2006-2007,2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,04dec13,dnb  written

*/

Component	DRV_IPROC_PCIE {
	NAME		iProc PCIe driver for VxBus
	SYNOPSIS	iProc PCIe driver
	MODULES		iProcPCIe.o
	INIT_RTN	iProcPCIeRegister();
	PROTOTYPE	void iProcPCIeRegister (void);
	_INIT_ORDER	hardWareInterFaceBusInit
	INIT_AFTER	INCLUDE_PLB_BUS
	REQUIRES	INCLUDE_VXBUS \
			INCLUDE_PLB_BUS
	_CHILDREN	FOLDER_DRIVERS
}

