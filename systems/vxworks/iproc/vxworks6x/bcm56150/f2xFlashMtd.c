/* f2xFlashMtd.c - f2xFlashMem MTD */

/*
 * Copyright (c) 2000-2001,2003,2007,2012 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01f,28mar12,tjf updated the copyright
01e,18jan07,tjf updated the copyright
01d,19feb03,tjf changed #include "stdLib.h" to #include "stdlib.h" for Solaris
01c,06dec03,tjf added INCLUDE_F2X_FORCE_WIDTH_READ switch to support H/W that
                  can only perform region-width-sized reads
01b,29jan01,tjf bugs fixed after running on real hardware
01a,19oct00,tjf writted by Ted Friedl, Wind River PS, Madison WI
*/

/*
DESCRIPTION
This library implements a TrueFFS MTD necessary to create one or
more TrueFFS devices from an f2xFlashMem flash group (represented
by a F2X_GID).

SPECIAL CONSIDERATIONS: MIXING MTDs
Other MTDs may be used with the f2xFlash MTD.  Because 
f2xFlashMtdIdentify() claims all sockets as its own, the identify
routines of any added MTDs must be respresented in the mtdTable[]
(see target/src/drv/tffs/tffsConfig.c) *before* the element
representing f2xFlashMtdIdentify().
*/

/* includes */

#include "vxWorks.h"
#include "stdioLib.h"
#include "stdlib.h"
#include "tffs/flflash.h"
#include "tffs/flsocket.h"
#include "tffs/flcustom.h"
#include "f2xFlashMem.h"
#include "vmLib.h"
#include "private/vmLibP.h"

/* defines */

typedef struct
    {
    char *   pCache;       /* map cache buffer */
    unsigned address;      /* starting group offset/CardAddress of map cache */
    int      validBytes;   /* number of valid bytes in map cache */
    int      cacheSize;    /* total size of the map cache */
    } MAP_CACHE;

#undef INCLUDE_F2X_MTD_DEBUG

/* locals */

#ifdef INCLUDE_F2X_FORCE_WIDTH_READ
LOCAL MAP_CACHE mapCacheArray[DRIVES];
#endif /* INCLUDE_F2X_FORCE_WIDTH_READ */

/* globals */

#ifdef INCLUDE_F2X_FORCE_WIDTH_READ
BOOL f2xFlashMtdMapCacheError = FALSE;      /* used to catch map cache errors */
#endif /* INCLUDE_F2X_FORCE_WIDTH_READ */

/******************************************************************************
*
* f2xFlashMtdMap - MTD map routine (see TrueFFS Programmer's Guide)
*
* RETURNS: Memory mapped address.
*/

LOCAL void FAR0 * f2xFlashMtdMap
    (
    FLFlash *   pVol,
    CardAddress address,
    int         length
    )
    {
    F2X_GID id = (F2X_GID)pVol->socket->serialNo;
    char *  mapAdrs;

#ifdef INCLUDE_F2X_FORCE_WIDTH_READ
    {
    MAP_CACHE * pMapCache = &mapCacheArray[pVol->socket->volNo];

    if (((unsigned)address < pMapCache->address) ||
        (((unsigned)address + length) >
         (pMapCache->address + pMapCache->validBytes)))
        {
        /* the desired mapping falls outside of the cache */

        if (length > pMapCache->cacheSize)
            {
            /* cache buffer is too small, replace it */

            if (pMapCache->pCache != NULL)
                free (pMapCache->pCache);

            /* get new cache buffer */

            pMapCache->pCache = malloc (length);
            if (pMapCache->pCache == NULL)
                {
                /* indicate an error occured */

                printf ("f2xFlashMtdMap: out of resources!\n");
                f2xFlashMtdMapCacheError = TRUE;

                pMapCache->cacheSize = 0;
                }
            else
                pMapCache->cacheSize = length;
            }
        
        if (length > pMapCache->cacheSize)
            {
            /* no resources: map the flash directly */

            mapAdrs = f2xGroupMap (id, (int)address);
            }
        else
            {
            /* read the flash into RAM and map it */

            f2xGroupGet (id, pMapCache->pCache, length, (int)address);
            mapAdrs = pMapCache->pCache;
            pMapCache->address = (unsigned)address;
            pMapCache->validBytes = length;
            }
        }
    else
        {
        /* we can satisfy this mapping without reading flash */

        mapAdrs = &(pMapCache->pCache[(unsigned)address - pMapCache->address]);
        }
    }
#else /* INCLUDE_F2X_FORCE_WIDTH_READ */

    mapAdrs = f2xGroupMap (id, (int)address);

#endif /* INCLUDE_F2X_FORCE_WIDTH_READ */

#ifdef INCLUDE_F2X_MTD_DEBUG
    printf ("f2xFlashMtdMap(0x%x, 0x%x, 0x%x) returns 0x%x\n",
            (unsigned)pVol, (unsigned)address, length, (unsigned)mapAdrs);
#endif /* INCLUDE_F2X_MTD_DEBUG */

    pVol->socket->remapped = TRUE;

    return(mapAdrs);
    }

/******************************************************************************
*
* f2xFlashMtdRead - MTD read routine (see TrueFFS Programmer's Guide)
*
* RETURNS: FLStatus
*/

LOCAL FLStatus f2xFlashMtdRead
    (
    FLFlash *   pVol,
    CardAddress address,
    void FAR1 * buffer,
    int         length,
    int         dummy
    )
    {
    F2X_GID id = (F2X_GID)pVol->socket->serialNo;
    STATUS  status;

#ifdef INCLUDE_F2X_MTD_DEBUG
    printf ("f2xFlashMtdRead(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
            (unsigned)pVol, (unsigned)address, (unsigned)buffer,
            length, dummy);
#endif /* INCLUDE_F2X_MTD_DEBUG */

    status = f2xGroupGet (id, (char *)buffer, length, (int)address);

    return (status == OK ? flOK : flReadFault);
    }

/******************************************************************************
*
* f2xFlashMtdWrite - MTD write routine (see TrueFFS Programmer's Guide)
*
* RETURNS: FLStatus
*/

LOCAL FLStatus f2xFlashMtdWrite
    (
    FLFlash *         pVol,
    CardAddress       address,
    const void FAR1 * buffer,
    int               length,
    FLBoolean         overwrite
    )
    {
    F2X_GID id = (F2X_GID)pVol->socket->serialNo;
    STATUS  status;

#ifdef INCLUDE_F2X_MTD_DEBUG
    printf ("f2xFlashMtdWrite(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
            (unsigned)pVol, (unsigned)address, (unsigned)buffer,
            length, overwrite);
#endif /* INCLUDE_F2X_MTD_DEBUG */

    status = f2xGroupProgram (id, (char *)buffer, length, (int)address);

#ifdef INCLUDE_F2X_FORCE_WIDTH_READ
    /* invalidate map cache */

    mapCacheArray[pVol->socket->volNo].validBytes = 0;
#endif /* INCLUDE_F2X_FORCE_WIDTH_READ */

    return (status == OK ? flOK : flWriteFault);
    }

/******************************************************************************
*
* f2xFlashMtdErase - MTD erase routine (see TrueFFS Programmer's Guide)
*
* RETURNS: FLStatus
*/

LOCAL FLStatus f2xFlashMtdErase
    (
    FLFlash * pVol,
    int       block,
    int       nblocks
    )
    {
    F2X_GID  id = (F2X_GID)pVol->socket->serialNo;
    int      blockSize = f2xGroupBlockSize(id);
    FLStatus flStatus = flOK;
    STATUS   status;
    int      offset;

#ifdef INCLUDE_F2X_MTD_DEBUG
    printf ("f2xFlashMtdErase(0x%x, 0x%x, 0x%x)\n",
            (unsigned)pVol, block, nblocks);
#endif /* INCLUDE_F2X_MTD_DEBUG */

    for (offset = block * blockSize;
         offset < (block + nblocks) * blockSize;
         offset += blockSize)
        {
        status = f2xGroupBlockErase (id, offset);

        if (status != OK)
            flStatus = flWriteFault;
        }

#ifdef INCLUDE_F2X_FORCE_WIDTH_READ
    /* invalidate map cache */

    mapCacheArray[pVol->socket->volNo].validBytes = 0;
#endif /* INCLUDE_F2X_FORCE_WIDTH_READ */

    return(flOK);
    }

/******************************************************************************
*
* f2xFlashMtdIdentify - MTD identify routine (see TrueFFS Programmer's Guide)
*
* NOTE: Other MTDs may be used with the f2xFlash MTD.  Because 
* f2xFlashMtdIdentify() claims all sockets as its own, the identify
* routines of any added MTDs must be respresented in the mtdTable[]
* (see target/src/drv/tffs/tffsConfig.c) *before* the element
* representing f2xFlashMtdIdentify().
*
* RETURNS: FLStatus
*/

FLStatus f2xFlashMtdIdentify
    (
    FLFlash vol
    )
    {
    F2X_GID  id = (F2X_GID)pVol->socket->serialNo;

    vol.type = 0x0000;
    vol.erasableBlockSize = f2xGroupBlockSize(id);
    vol.chipSize = f2xGroupSize(id);
    vol.noOfChips = 1;
    vol.interleaving = 1;
    vol.map = f2xFlashMtdMap;
    vol.read = f2xFlashMtdRead;
    vol.write = f2xFlashMtdWrite;
    vol.erase = f2xFlashMtdErase;

    return(flOK);
    }
