/* bcm1250Dec21x40End.c - BCM1250 overrides of dec21x40End driver */

/* Copyright 2002 Wind River Systems, Inc. */

/* $Id: bcm1250Dec21x40End.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf  written.
*/

/*
DESCRIPTION
The following macros use the BCM1250 match-bit-lane addressing
mode so that no swap (PCISWAP) of the value to be read or written
is required.
*/

#include "bcm1250.h"
#include "drv/pci/pciIntLib.h"

#define BCM1250_MATCH_BITS   0x20000000

#define CSR_ADDR(csr)                                                   \
        ((pDrvCtrl->devAdrs | BIT_ENDIAN_OFFSET) + DECPCI_REG_OFFSET*(csr))

#define DEC_CSR_READ(csr)                                               \
        hs_read32 (MIPS_PHYS_TO_XKSEG_UNCACHED(CSR_ADDR(csr)))

#define DEC_CSR_WRITE(csr,val)                                          \
        hs_write32 (MIPS_PHYS_TO_XKSEG_UNCACHED(CSR_ADDR(csr)), (val))

#define SYS_INT_CONNECT(pDrvCtrl,rtn,arg,pResult)                       \
        *pResult = pciIntConnect((VOIDFUNCPTR *)INUM_TO_IVEC (pDrvCtrl->ivec),\
                               (rtn), (int)(arg));

#define SYS_INT_DISCONNECT(pDrvCtrl,rtn,arg,pResult)                    \
        *pResult = pciIntDisconnect((VOIDFUNCPTR *)INUM_TO_IVEC (pDrvCtrl->ivec),\
                               (rtn));

#define SYS_INT_ENABLE(pDrvCtrl)                                        \
    ((void)0)

#define SYS_INT_DISABLE(pDrvCtrl)                                       \
    ((void)0)

#include "end/dec21x40End.c"
