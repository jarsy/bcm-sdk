/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * VxWorks 5.x END OS Layer
 *
 * Copyright(c) 2001 Broadcom Corp.
 * $Id: vx_osl.c,v 1.3 2007/10/08 22:31:08 iakramov Exp $
 */

#include <typedefs.h>
#include <vx_osl.h>
#include <cacheLib.h>
#include <bcmutils.h>
#ifndef MSI
#include <drv/pci/pciConfigLib.h>
#endif
#include <logLib.h>
#include "tickLib.h"
#include "sysLib.h"
#include "taskLib.h"


#define	PKTRSV	32		/* #bytes to reserve at the front of each cluster buffer */

#define	SAMEPAGE(x, y)		(((ulong)(x) & ~4095) == ((ulong)(y) & ~4095))

static int loop_us=200;	/* default until calibration done in osl_init */

void
osl_init(void)
{	uint ticks_per_us, tick1=1, tick2=0, n;
	volatile int i=1000000;
	static bool init_done=FALSE;

	if (init_done) 
		return;

	/*** calibrate the delay loop ***/

	/* get the ticks per us */
	ticks_per_us = 1000000/sysClkRateGet();

	taskLock();

	/* get the curr tick count (retry if wrapped) */
	while(tick2 < tick1) {
		tick1 = tickGet();

		/* Loop 1000000 times */
		while (i--);

		/* get new tick count */
		tick2 = tickGet();
	}

	taskUnlock();

	/* calculate loop count per uS */
	n = tick2-tick1;
	if (!n) n=1;
	loop_us = 1000000/(n*ticks_per_us);
	
	init_done = TRUE;
}

void*
osl_pktget(void *drv, uint len, bool send)
{
	END_OBJ *end;
	M_BLK_ID m;

	/* size requested should fit in our cluster buffer */
	ASSERT(len < (VXCLSIZE - sizeof (CL_BUF) - PKTRSV));

	/* drv is a pointer to an END_OBJ in disguise */
	end = (END_OBJ*)drv;

	/* older chips cannot dma address buffers which cross 4Kbyte boundaries */
	if ((m = netTupleGet(end->pNetPool, (len + PKTRSV), M_DONTWAIT, MT_DATA, FALSE))) {

		ASSERT(SAMEPAGE(m->mBlkHdr.mData, (m->mBlkHdr.mData + len - 1))); 
		/* reserve a few bytes */
		m->mBlkHdr.mData += PKTRSV;
		m->mBlkHdr.mLen = len;

		/* ensure the cookie field is cleared */ 
		PKTSETCOOKIE(m, NULL);
	}
	
	return ((void*) m);
}				  

void
osl_pktfree(void *drv, M_BLK_ID m, bool send)
{
	netMblkClChainFree(m);
}

uchar*
osl_pktpush(void *drv, M_BLK_ID m, uint nbytes)
{
	ASSERT(M_HEADROOM(m) >= (int)nbytes);
	m->mBlkHdr.mData -= nbytes;
	m->mBlkHdr.mLen += nbytes;
	return (m->mBlkHdr.mData);
}

uchar*
osl_pktpull(void *drv, M_BLK_ID m, uint nbytes)
{
	ASSERT((int)nbytes <= m->mBlkHdr.mLen);
	m->mBlkHdr.mData += nbytes;
	m->mBlkHdr.mLen -= nbytes;
	return (m->mBlkHdr.mData);
}

void*
osl_pktdup(void *drv, void *p)
{
	END_OBJ *end;
	M_BLK_ID m;
	int len = pkttotlen(drv, p);

	/* drv is a pointer to an END_OBJ in disguise */
	end = (END_OBJ*)drv;

	/* dont use a real dup until PR14233 is fixed 
	m = netMblkChainDup(end->pNetPool, (M_BLK_ID)p, 0, M_COPYALL, M_DONTWAIT);	 */
	
	/* alloc a new mbuf */
	if ((m = PKTGET(drv, len, TRUE)) == NULL)
		return (NULL);

	/* copy data into mbuf */
	pktcopy(drv, p, 0, len, (uchar*)PKTDATA(drv, m));
	ASSERT(len == PKTLEN(drv,  m));

	return (m);
}

void
osl_assert(char *exp, char *file, int line)
{
	char tempbuf[255];

	sprintf(tempbuf, "\nassertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	logMsg(tempbuf, 0,0,0,0,0,0);
	abort();
}

void*
osl_dma_alloc_consistent(void *dev, uint size, void **pap)
{
	void *va;

	va = cacheDmaMalloc(size);
	*pap = (void *)CACHE_DMA_VIRT_TO_PHYS(va);
	return (va);
}

void
osl_dma_free_consistent(void *dev, uint size, void *va, void *pa)
{
	cacheDmaFree(va);
}

void*
osl_dma_map(void *dev, void *va, uint size, uint direction)
{
	if (direction == DMA_TX)
		cacheFlush(DATA_CACHE, va, size);
	else
		cacheInvalidate(DATA_CACHE, va, size);

	return ((void*)CACHE_DMA_VIRT_TO_PHYS(va));
}

void
osl_delay(uint us)
{
	volatile uint n;

	n = us * loop_us;
	while (n--)
		;
}

