/* sysBcm570xEnd.c - system configuration module for BCM570x END */

/* $Id: sysBcm570xEnd.c,v 1.3 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------
01b,28jun02,jmb  set cache funcs struct
01a,29may02,jmb  created
*/

/*
DESCRIPTION
This is the configuration module for the VxWorks BCM570x (bc) END
driver.  It has been adapted for "BMW".
*/

#if (defined(INCLUDE_NETWORK) && defined(INCLUDE_BCM570XEND) \
     && defined (INCLUDE_END))

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "end.h"
#include "config.h"

#include "../../../drv/bcm570x/bcm570xEnd.h"

/* defines */

/* forward declarations */

/* typedefs */

/* locals */

IMPORT END_OBJ* bcm570xEndLoad (char *);

/******************************************************************************
*
* sysBcm570xCacheFuncsGet - Get flush/invalidate routine for driver
*
* RETURNS: N/A
*
*/

void sysBcm570xCacheFuncsGet
    (
    CACHE_FUNCS *pCacheFuncs
    )
    {
    *pCacheFuncs = cacheDmaFuncs;
    return;
    }


/******************************************************************************
*
* sysBcm570xEndLoad - board specific initialization for BCM570x
*
* RETURNS: pointer to END object or ERROR.
*
* SEE ALSO: bcm570xEndLoad()
*/

END_OBJ * sysBcm570xEndLoad
    (
    char * pParamStr,   /* ptr to initialization parameter string */
    void * unused       /* BSP-specific param */
    )
    {
    END_OBJ * pEnd;

    if (strlen (pParamStr) == 0)
        {
        /*
         * muxDevLoad() calls us twice.  If the string is
         * zero length, then this is the first time through
         * this routine, so we just return.
         */

        pEnd = bcm570xEndLoad (pParamStr);
        }

    else
        {
        /*
        * This cacheDma stuff is here to let the driver know that memory is
        * write coherent.  It doesn't have to do flushes or invalidates since
        * the PPC 8245 does bus snooping.
        */

        cacheDmaFuncs.flushRtn = NULL;
        cacheDmaFuncs.invalidateRtn = NULL;
        _func_bcm570xEnetAddrGet = sysEnetAddrGet;

        if ((pEnd = bcm570xEndLoad (pParamStr)) == (END_OBJ *)ERROR)
            {
            printf ("Error: bcm570x device failed bcm570xEndLoad routine.\n");
            }
        }

    return (pEnd);
    }

#endif
