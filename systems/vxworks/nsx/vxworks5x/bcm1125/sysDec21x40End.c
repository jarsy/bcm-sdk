/* bcm1250Dec21x40End.c - Tulip configuration module for BCM1250 */

/* Copyright 2002 Wind River Systems, Inc. */

/* $Id: sysDec21x40End.c,v 1.3 2011/07/21 16:14:44 yshtil Exp $
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
This code performs the dynamic parameterization of a Tulip driver (dc)
according to the result of PCI configuration.
This technique of 'just-in-time' parameterization allows driver
parameter values to be declared as any other defined constants rather 
than as static strings. 
*/
 
/* includes */

#include "vxWorks.h"
#include "config.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "end.h"
#include "cacheLib.h"
#include "drv/end/dec21x40End.h"
#include "drv/pci/pciConfigLib.h"

/* defines */

#define VENDOR_ID_DEC   0x1011
#define DEVICE_ID_21040 0x0002
#define DEVICE_ID_21041 0x0014
#define DEVICE_ID_21140 0x0009
#define DEVICE_ID_21143 0x0019

#define DEC_CFG_CBMA      0x14
#define DEC_CFG_CFIT      0x3c
#define DEC_CFG_CFDD      0x40

/* imports */

IMPORT END_OBJ *dec21x40EndLoad (char *);

/******************************************************************************
*
* sysDec21x40EndLoad - load (create) a Tulip device
*
* This routine loads the de device with initial parameters specified by
* values assigned during PCI configuration.
*
* RETURNS: pointer to END object or ERROR.
*
* Note that the addresses specified for DMA access select the
* match-byte-lanes policy; thus the 21x4x BE bit in CSR0 (DEC_USR_BE)
* should not be set, even for big-endian host systems.
* Configuration and device CSR accesses use addresses that select
* match-bit-lanes policy to avoid software byte swapping in manipulation
* of the corresponding registers.
*
* SEE ALSO: dec21x40EndLoad()
*/
 
END_OBJ *sysDec21x40EndLoad
    (
    char *pParamStr,   /* ptr to initialization parameter string */
    void *hwUnit       /* Tulip unit number */
    )
    {
      /* 
       *  The format of the parameter string should be:
       * "<unit number>:<device addr>:<PCI addr>:"
       * "<ivec>:<ilevel>:<numRds>:<numTds>:<mem base>:<mem size>:"
       * "<user flags>:<phyAddr>:<pPhyTbl>:<phyFlags>:<offset>"
       */

    int unit = (int) hwUnit;
    char *cp;
    char paramStr [END_INIT_STR_MAX];   /* from end.h */
    static char dec21x40ParamTemplate [] = 
      "%#x:0:%#x:0:-1:-1:-1:0:%#x::::2";
    int busNo, deviceNo, funcNo;
    UINT32 flags;
    UINT32 csrAdrs;
    UINT8  intLine;
    UINT32 cfdd;

    END_OBJ *pEnd;

    if (strlen (pParamStr) == 0)
        {
        /* 
         * muxDevLoad() calls us twice.  If the string is
         * zero length, then this is the first time through
         * this routine, so we just return.
         */
        pEnd = dec21x40EndLoad (pParamStr);
        }
    else
	{
	if (pciFindDevice (VENDOR_ID_DEC, DEVICE_ID_21143, unit,
			   &busNo, &deviceNo, &funcNo) == OK)
	    {
	    flags = DEC_USR_21143;
	    }
	else
	if (pciFindDevice (VENDOR_ID_DEC, DEVICE_ID_21140, unit,
			   &busNo, &deviceNo, &funcNo) == OK)
	    {
	    flags = DEC_USR_21140;
	    }
	else
	if (pciFindDevice (VENDOR_ID_DEC, DEVICE_ID_21041, unit,
			   &busNo, &deviceNo, &funcNo) == OK)
	    {
	    flags = 0;
	    }
	else
	    {
	    return (END_OBJ *)ERROR;
	    }

	pciConfigInLong (busNo, deviceNo, funcNo, DEC_CFG_CBMA, &csrAdrs);
	pciConfigInByte (busNo, deviceNo, funcNo, DEC_CFG_CFIT, &intLine);

	/* Make sure that chip is out of sleep mode. */
	pciConfigInLong (busNo, deviceNo, funcNo, DEC_CFG_CFDD, &cfdd);
	pciConfigOutLong (busNo, deviceNo, funcNo, DEC_CFG_CFDD, 0);
	pciConfigInLong (busNo, deviceNo, funcNo, DEC_CFG_CFDD, &cfdd);

        /*
         * On the second pass though here, we actually create 
         * the initialization parameter string on the fly.   
         * Note that we will be handed our unit number on the 
         * second pass through and we need to preserve that information.
         * So we use the unit number handed from the input string.
         */

        cp = strcpy (paramStr, pParamStr); /* cp points to paramStr */

        /* Now, we advance cp, by finding the end the string */

        cp += strlen (paramStr);
        
        /* finish off the initialization parameter string */

	flags |= DEC_USR_CAL_08 | DEC_USR_PBL_32 | DEC_USR_RML | DEC_USR_SF;
	sprintf (cp, dec21x40ParamTemplate, csrAdrs, intLine, flags);
        if ((pEnd = dec21x40EndLoad (paramStr)) == (END_OBJ *)ERROR)
	    {
            printf ("Error: dec21x40 device failed EndLoad routine.\n");
	    }
	}

    return (pEnd);
    }

/*
 * Assign a user-supplied Ethernet address if SROM lookup fails or
 * DEC_USR_XEA is specified.
 */

STATUS sysDec21x40EnetAddrGet 
    (
    int unit, 
    char *enetAdrs
    )
    {
    enetAdrs[0] = 0x00;
    enetAdrs[1] = 0x02;
    enetAdrs[2] = 0x4c;
    enetAdrs[3] = 0xde;
    enetAdrs[4] = 0x00 + (unit / 256);
    enetAdrs[5] = 0x00 + (unit % 256);

    return OK;
    }

