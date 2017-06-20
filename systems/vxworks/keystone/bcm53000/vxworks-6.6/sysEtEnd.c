/* sysEtnd.c - system configuration module for BCM47xx et END */

/* $Id: sysEtEnd.c,v 1.3 2011/07/21 16:14:28 yshtil Exp $
modification history
--------------------
01a,02aug02,jmb  created
*/

/*
DESCRIPTION
This is the configuration module for the VxWorks BCM4702 (et) END
driver.  It has been adapted for "MBZ".
*/

#if (defined(INCLUDE_NETWORK) && defined(INCLUDE_END) && \
    (defined(INCLUDE_ET0_END) || defined(INCLUDE_ET1_END)))

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "end.h"
#include "config.h"

/* defines */

/* forward declarations */

/* typedefs */

/* locals */

IMPORT END_OBJ* et_load (char *);
IMPORT END_OBJ* etv_load (char *);
IMPORT END_OBJ* wl_load (char *);

/******************************************************************************
*
* sysEtEndLoad - board specific initialization for BCM4702 internal ethernet
*
* RETURNS: pointer to END object or ERROR.
*
* SEE ALSO: bcm570xEndLoad()
*/

END_OBJ * sysEtEndLoad
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

        pEnd = et_load (pParamStr);
        }

    else
        {
        if ((pEnd = et_load (pParamStr)) == (END_OBJ *)ERROR)
            {
            printf ("Error: et device failed et_load routine.\n");
            }
        }

    return (pEnd);
    }

#ifdef INCLUDE_ETV0_END
/******************************************************************************
*
* sysEtvEndLoad - board specific initialization for virtual END driver
*
* RETURNS: pointer to END object or ERROR.
*
* SEE ALSO: bcm570xEndLoad()
*/

END_OBJ * sysEtvEndLoad
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

        pEnd = etv_load (pParamStr);
        }

    else
        {
        if ((pEnd = etv_load (pParamStr)) == (END_OBJ *)ERROR)
            {
            printf ("Error: etv device failed etv_load routine.\n");
            }
        }

    return (pEnd);
    }
#endif /* INCLUDE_ETV0_END */

#ifdef INCLUDE_WL_END
/******************************************************************************
*
* sysWlEndLoad - board specific initialization for wireless END driver
*
* RETURNS: pointer to END object or ERROR.
*
* SEE ALSO: bcm570xEndLoad()
*/

END_OBJ * sysWlEndLoad
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

        pEnd = wl_load (pParamStr);
        }

    else
        {
        if ((pEnd = wl_load (pParamStr)) == (END_OBJ *)ERROR)
            {
            printf ("Error: wl device failed wl_load routine.\n");
            }
        }

    return (pEnd);
    }
#endif

#endif

