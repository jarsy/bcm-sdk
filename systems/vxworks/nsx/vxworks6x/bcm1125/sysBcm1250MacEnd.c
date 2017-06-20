/* sysBcm1250MacEnd.c - system configuration module for BCM1250 MAC END */
 
/* $Id: sysBcm1250MacEnd.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * Copyright (c) 2002-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
*/

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
01h,03aug05,dr   added #ifdef INCLUDE_END for scalability.
01g,08nov04,mdo  Documentation fixes for apigen
01f,05may04,agf  fix compiler warnings
01e,31may02,pgh  Remove references to DMA channel 1.
01d,26mar02,tlc  Remove compiler warnings.
01c,07mar02,kab  SPR 70817: *EndLoad returns NULL on failure
01b,04jan02,agf  add nvRAM support supplied by Z.Chen
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

INCLUDE FILES:
*/
 
/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <end.h>
#include "config.h"
#include <x1240RtcEeprom.h>

/* defines */

#define SBE_IVECNUM            IV_INT3_VEC
#define SBE_USR_FLAGS          0
#define SBE_NUM_RDS0           128 
#define SBE_NUM_TDS0           128 
 
#ifdef	INCLUDE_END
/* externals */

IMPORT END_OBJ * bcm1250MacEndLoad (char *);
IMPORT STATUS    nvramEnvGet (char *name, char *string, int strLen);

/* forward declarations */

LOCAL int parse_xdigit (char str);
LOCAL int parse_hwaddr (char *str, uint8_t *hwaddr);


/******************************************************************************
*
* sysBcm1250MacEndLoad - load the BCM1250 MAC device
*
* This routine loads the BCM 1250 device with initial parameters specified by
* values given in the BSP configuration files.
*
* RETURNS: pointer to END object or NULL.
*
* ERRNO
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
       * <numRds0>:<numTds0>"
       */


    char * cp;
    char paramStr [END_INIT_STR_MAX];   /* from end.h */
    static char bcm1250MacParamTemplate [] = "%d:%d:%#x:%d:%d"; 

    END_OBJ * pEnd;

    if (strlen (pParamStr) == 0)
        {
        /* 
         * muxDevLoad() calls us twice.  If the string is
         * zero length, then this is the first time through
         * this routine, so we just return.
         */
       /* printf("mac load called 1st time\n");*/
        pEnd = bcm1250MacEndLoad (pParamStr);
        }

    else
	{

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
		 SBE_NUM_TDS0);

        if ((pEnd = bcm1250MacEndLoad (paramStr)) == (END_OBJ *)NULL)
	    {
            printf ("Error: BCM1250 Mac device failed bcm1250MacEndLoad routine.\n");
	    }
	}

    return (pEnd);
    }

/******************************************************************************
*
* sysBcm1250MacEnetAddrGet - get the hardware Ethernet address
*
* This routine gets the hardware ethernet address from nvRAM and passes it
* back to the bcm1250 END.
*
* RETURNS: N/A
*
* ERRNO
*
* SEE ALSO: bcm1250MacEndLoad()
*/
#if 1
void sysBcm1250MacEnetAddrGet
    (
    int unit, 
    uint8_t * enetAdrs
    )
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
#else
void sysBcm1250MacEnetAddrGet
    (
    int unit, 
    uint8_t * enetAdrs
    )
    {
    char hwAddr[18];
    char envName[12];

    /* 
     * following strings are field descriptors used by Broadcom's CFE
     * refer to CFE documentation for full details
     */
 
    if (unit == 0)
       strcpy(envName, "ETH0_HWADDR");
    else
       strcpy(envName, "ETH1_HWADDR");
 
    if (nvramEnvGet(envName, hwAddr, 17) == OK)
        if (parse_hwaddr(hwAddr, enetAdrs) == 0)
            return;

    enetAdrs[0] = 0x00;
    enetAdrs[1] = 0x02;
    enetAdrs[2] = 0x4c;
    enetAdrs[3] = 0xff;
    enetAdrs[4] = 0x00;
    switch (unit)
        {
        case 0:
        enetAdrs[5] = 0x20;
        break;
        case 1:
        enetAdrs[5] = 0x21;
        break;
        case 2:
        enetAdrs[5] = 0x22;
        break;
        }

    }

/**********************************************************************
* parse_xdigit - parse a hex digit
*
* This routine parses a hex digit and returns its value.
*
* RETURNS: hex value, or -1 if invalid
*
* ERRNO
*/
LOCAL int parse_xdigit
    (
    char str
    )
    {
    int digit;
 
    if ((str >= '0') && (str <= '9'))
        digit = str - '0';
    else if ((str >= 'a') && (str <= 'f'))
        digit = str - 'a' + 10;
    else if ((str >= 'A') && (str <= 'F'))
        digit = str - 'A' + 10;
    else
        return -1;
 
    return digit;
    }


/**********************************************************************
* parse_hwaddr - convert a string to Ethernet Address
*
* This routine converts a string in the form xx:xx:xx:xx:xx:xx into 
* a 6-byte Ethernet address.
*
* RETURNS: 0 if ok, else -1
*
* ERRNO
*/
LOCAL int parse_hwaddr
    (
    char *str,
    uint8_t *hwaddr
    )
    {
    int digit1,digit2;
    int idx = 6;
 
    while (*str && (idx > 0))
        {
        digit1 = parse_xdigit(*str);
        if (digit1 < 0)
            return -1;
        str++;
        if (!*str)
            return -1;

        if ((*str == ':') || (*str == '-'))
            {
            digit2 = digit1;
            digit1 = 0;
            }
        else
            {
            digit2 = parse_xdigit(*str);
            if (digit2 < 0)
                return -1;
            str++;
            }
 
        *hwaddr++ = (digit1 << 4) | digit2;
        idx--;
 
        if (*str == '-')
            str++;
        if (*str == ':')
            str++;
        }
    return 0;
}
#endif
#endif	/* INCLUDE_END */
