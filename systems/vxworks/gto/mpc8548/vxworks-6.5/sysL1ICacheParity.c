/* sysL1ICacheParity.c - Motorola ads 85xx board L1 instruction cache parity */

/* $Id: sysL1ICacheParity.c,v 1.2 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2003-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement. No license to Wind River intellectual property rights
 * is granted herein. All rights not licensed by Wind River are reserved
 * by Wind River.
 */
 
#include "copyright_wrs.h"

/*
modification history
--------------------
01d,23mar05,mdo  Documentation fixes for apigen
01c,18aug04,dtr  Update names to reflect usage change.
01b,10jun04,dtr  Need to disable instruction cache before modifying parity
                 bit.
01a,10jun04,dtr  Created. 
*/
 
/*
DESCRIPTION

INCLUDE FILES:
*/

UINT instrParityCount=0;
IMPORT void sysIParityHandler();
IMPORT void sysIParityHandlerEnd();
#ifdef INCLUDE_SHOW_ROUTINES
void l1InstrParityErrorShow();
#endif

/*************************************************************************
*
* installL1ErrataParityErrorRecovery - install L1 parity error handler
*
* This routine installs the L1 parity error handler. The handler code is 
* with the E500 cache library. Handler not used until IVOR1 set. 
*
* RETURNS: status
*
* ERRNO
*
* \NOMANUAL
*/
STATUS installL1ICacheParityErrorRecovery(void)
    {
    int key;
    STATUS status;
    GENERIC_LAYERED_EXC_INSTALL_DESC mchkParityDesc;
    static char installErrMsg[] = "L1 Parity exception handler install has failed";
    mchkParityDesc.funcStart = &sysIParityHandler;
    mchkParityDesc.funcEnd = &sysIParityHandlerEnd;
    mchkParityDesc.excOffset = _EXC_OFF_L1_PARITY;
    mchkParityDesc.errExcOffset = _EXC_OFF_MACH;
    mchkParityDesc.hdlrBase = NULL;
    mchkParityDesc.hdlrCodeBase = NULL;
    mchkParityDesc.forceBase = TRUE;
    mchkParityDesc.excMsg = &installErrMsg[0];
    /* Cautious should only be started from single thread */
    key = intLock();
    status = genericLayeredExcHdlrInstall(&mchkParityDesc);
    intUnlock(key);

    vxIvor1Set(_EXC_OFF_L1_PARITY);

    cacheDisable(INSTRUCTION_CACHE);
    vxL1CSR1Set(vxL1CSR1Get()|_PPC_L1CSR_CPE);
    cacheEnable(INSTRUCTION_CACHE);

    return status;
    }


#ifdef INCLUDE_SHOW_ROUTINES
/***************************************************************************
*
* l1InstructionParityErrorShow - dump L1 instruction parity errors
*
* This routine prints out number of L1 instruction parity 
* errors recovered from.
*
* RETURNS: N/A
*
* ERRNO
*/

void l1InstrParityErrorShow(void)
    {
    printf("Number of L1 Instruction Parity Errors : %d\n",instrParityCount);
    }

#endif /* INCLUDE_SHOW_ROUTINES */
