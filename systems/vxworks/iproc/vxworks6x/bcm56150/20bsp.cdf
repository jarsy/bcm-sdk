/* 20bsp.cdf - BSP-specific component descriptor file */

/*
 * Copyright (c) 2010-2011,2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,01nov13,dnb  created from arm_a9_ctx version 01b 
*/

/*
DESCRIPTION
This file contains descriptions for this BSP components.
*/

Bsp brcm_hurricane2 {
    NAME        board support package
    CPU         ARMARCH7
    ENDIAN      little be8
    FP          vector soft
    MP_OPTIONS
    REQUIRES    INCLUDE_KERNEL
}

Parameter RAM_HIGH_ADRS {
    NAME        Bootrom Copy region
    DEFAULT     0x69000000
}

Parameter RAM_LOW_ADRS {
    NAME        Runtime kernel load address
    DEFAULT     0x68100000
}

/*
 * VX_SMP_NUM_CPUS is a SMP parameter only and only available for SMP
 * builds. Due to a limitation of the project tool at the time this
 * parameter is created where the tool can not recognize the ifdef SMP
 * selection, this parameter is set up such that _CFG_PARAMS is not
 * specified here. In the 00vxWorks.cdf file, where the parameter
 * VX_SMP_NUM_CPUS is defined, the _CFG_PARAMS is specified only for
 * VxWorks SMP. Hence the redefining of VX_SMP_NUM_CPUS here should only
 * override the value and not the rest of the properties. And for UP, the
 * parameter is ignored since the parameter is not tied to any component
 * (_CFG_PARAMS is not specified).
 */

Parameter VX_SMP_NUM_CPUS {
        NAME            Number of CPUs available to be enabled for VxWorks SMP
        TYPE            UINT
        DEFAULT         1
}

Component	INCLUDE_PNOR_FLASH {
	NAME		iProc PNOR Flash Support for TFFS
	SYNOPSIS	iProc PNOR Flash
}


