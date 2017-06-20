/* $Id: sysBcm1250MacEnd.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * sysBcm1250MacEnd.c - system configuration module for BCM1250 MAC END
 */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */
 
/*
modification history
--------------------
01a,15nov01,agf  written 
*/


/*
DESCRIPTION
This is the WRS-supplied configuration module for the VxWorks 
END driver for the Broadcom BCM1250 chip
 
It performs the dynamic parameterization of the bcm1250MacEnd driver.
This technique of 'just-in-time' parameterization allows driver
parameter values to be declared as any other defined constants rather 
than as static strings. 
*/
 
/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "end.h"
#include "config.h"

/* defines */

#define SBE_IVECNUM            IV_INT3_VEC
#define SBE_USR_FLAGS          0
#define SBE_NUM_RDS0           64 
#define SBE_NUM_TDS0           64 
#define SBE_NUM_RDS1           16 
#define SBE_NUM_TDS1           16
 
/* imports */

IMPORT END_OBJ * bcm1250MacEndLoad (char *);
IMPORT STATUS sysEnetAddrGet (char *dev, int unit, unsigned char *pMac);

/******************************************************************************
*
* sysBcm1250MacEndLoad - load (create) BCM1250 MAC (sbe) device
*
* This routine loads the ene device with initial parameters specified by
* values given in the BSP configuration files (config.h).
*
* RETURNS: pointer to END object or ERROR.
*
* SEE ALSO: bcm1250MacEndLoad()
*/
 
END_OBJ * sysBcm1250MacEndLoad
    (
    char * pParamStr,   /* ptr to initialization parameter string */
    void * macHwUnit    /* Hardware MAC unit number */
    )
    {
      /* 
       *  The format of the parameter string should be:
       * "<hwunitnum>:<ivecnum>:<user_flags>:
       * <numRds0>:<numTds0>:<numRds1>:<numTds1>"
       */


    char * cp;
    char paramStr [END_INIT_STR_MAX];   /* from end.h */
    static char bcm1250MacParamTemplate [] = 
      "%d:%d:%#x:%d:%d:%d:%d"; 

    END_OBJ * pEnd;

    if (strlen (pParamStr) == 0)
        {
        /* 
         * muxDevLoad() calls us twice.  If the string is
         * zero length, then this is the first time through
         * this routine, so we just return.
         */
        /* printf("mac load called 1st time\n"); */
        pEnd = bcm1250MacEndLoad (pParamStr);
        }

    else
	{
#if 0
        printf("sysBcm1250MacEndLoad - pParamStr %s\n", pParamStr);
        printf("sysBcm1250MacEndLoad - MAC hw unit number %d\n", (int)macHwUnit);
#endif

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

	sprintf (cp, bcm1250MacParamTemplate, 
		 (int) macHwUnit,
		 SBE_IVECNUM,
		 SBE_USR_FLAGS,
		 SBE_NUM_RDS0,
		 SBE_NUM_TDS0,
		 SBE_NUM_RDS1,
		 SBE_NUM_TDS1);

        if ((pEnd = bcm1250MacEndLoad (paramStr)) == (END_OBJ *)ERROR)
	    {
            printf ("Error: BCM1250 Mac device failed bcm1250MacEndLoad routine.\n");
	    }
	}

    return (pEnd);
    }

void sysBcm1250MacEnetAddrGet(int unit, char * enetAdrs)
{
    if (sysEnetAddrGet("sbe", unit, (unsigned char*)enetAdrs) == ERROR)
        {
        printf ("Error: Invalid Mac address - using default.\n");
        enetAdrs[0] = 0x40;
        enetAdrs[1] = 0x00;
        enetAdrs[2] = 0x0a;
        enetAdrs[3] = 0x15;
        enetAdrs[4] = 0x40;
        enetAdrs[5] = 0x12 + unit;
        }
}
