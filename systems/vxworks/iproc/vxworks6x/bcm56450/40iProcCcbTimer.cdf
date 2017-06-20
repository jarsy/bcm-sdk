/* 40iProcCcbTimer.cdf - Component configuration file */

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
01a,01nov13,dnb  written

*/

Component	DRV_TIMER_IPROC_CCB {
	NAME		iProc Ccb  timer driver for VxBus
	SYNOPSIS	iProc Ccb  for Timer Driver support
	MODULES		iProcCcbTimer.o
	INIT_RTN	iProcCcbTimerDrvRegister();
	PROTOTYPE	void iProcCcbTimerDrvRegister (void);
	_INIT_ORDER	hardWareInterFaceBusInit
	INIT_AFTER	INCLUDE_PLB_BUS
	REQUIRES	INCLUDE_VXBUS \
			INCLUDE_PLB_BUS \
			INCLUDE_TIMER_SYS
	_CHILDREN	FOLDER_DRIVERS
}

