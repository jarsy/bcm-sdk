/* sysCache.c - secondary (L2) cache library for the Falcon
 * Copyright 1984-1997 Wind River Systems, Inc. 
 * Copyright 1996 Motorola, Inc. 


modification history
--------------------
01a,08jul97,dat  written (from mv2603/sysCache.c, ver 01b)
*/

/* $Id: sysCache.c,v 1.2 2011/07/21 16:14:08 yshtil Exp $
DESCRIPTION

This library provides L2 cache support, and is derived from
code supplied by Motorola.

The L2 cache should always be set up in writethrough mode.  If the
L2 cache jumper is present on the MVME2600 series board, it should be 
removed.

The driver only sets or clears one bit at a time in the System
External Cache Control register, per the Genesis 2 programming
guide.

The flush bit in the System External Cache Control register should
not be used.

*/

/* includes */

#include "vxWorks.h"
#include "config.h"

#if defined(INCLUDE_CACHE_SUPPORT) && defined(INCLUDE_CACHE_L2)

/******************************************************************************
*
* sysL2CacheInit - initialize the L2 cache 
*
* This routine initializes and enables L2 cache support.  It should be
* called at startup, before sysL2CacheEnable(), to avoid invalidating the L2
* tags.
*
* RETURNS: OK, or ERROR if cache is not present or not supported.
*
* SEE ALSO: sysL2CacheDisable(), sysL2CacheEnable()
*
*/ 

STATUS sysL2CacheInit (void)
    {


    sysL2CacheDisable ();	/* disable and invalidate all tags */
    return (OK);
    }

/******************************************************************************
*
* sysL2CacheDisable - disable the L2 cache
*
* This routine disables the L2 cache if it was previously initialized using 
* sysL2CacheInit().  Calling this routine invalidates the L2 tag bits.
*
* RETURNS: N/A
*
* SEE ALSO: sysL2CacheEnable(), sysL2CacheInit()
*/ 

void sysL2CacheDisable (void)
    {
    unsigned char	regVal;
    int		  	temp;
    int			i;

    }

/******************************************************************************
*
* sysL2CacheEnable - enable the L2 cache
* 
* This routine enables the L2 cache by calling sysL2CacheInit().  It checks
* for the presence of an L2 cache and ensures that the cache is disabled.
*
* RETURNS: N/A
*
* SEE ALSO: sysL2CacheDisable(), sysL2CacheInit()
*/ 

void sysL2CacheEnable (void)
    {
    }

#endif	/* INCLUDE_CACHE_SUPPORT */
