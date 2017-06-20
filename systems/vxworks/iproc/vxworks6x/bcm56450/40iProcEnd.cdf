/* 40iProcEnd.cdf - Component configuration file */

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

Component	DRV_VXBEND_IPROC {
	NAME		iProc VxBus Enhanced Network Driver
	SYNOPSIS	iProc VxbEnd
	MODULES		iProcVxbEnd.o
	PROTOTYPE	void iProcVxbEndRegister (void);
	_CHILDREN	FOLDER_DRIVERS
	_INIT_ORDER	hardWareInterFaceBusInit
	INIT_RTN	iProcVxbEndRegister();
	REQUIRES	INCLUDE_PLB_BUS \
			INCLUDE_GENERICPHY \
			INCLUDE_PARAM_SYS \
                        INCLUDE_DMA_SYS
	INIT_AFTER	INCLUDE_PLB_BUS
}


