/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * vxWorks 5.x device driver private data structure for
 * Broadcom BCM44XX and BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright(c) 2000 Broadcom Corp.
 * $Id: et_priv.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _et_priv_h_
#define _et_priv_h_

#include <etc.h>

/* private chip state */
struct bcm4xxx {
	void 		*et;		/* pointer to et private state */
	etc_info_t	*etc;		/* pointer to etc public state */

	bcmenetregs_t	*regs;		/* pointer to chip registers */
	void 		*dev;		/* device handle */

	void 		*etphy;		/* pointer to et for shared mdc/mdio contortion */

	uint32		intstatus;	/* saved interrupt condition bits */
	uint32		intmask;	/* current software interrupt mask */

	void 		*di;		/* dma engine software state */

	bool		mibgood;	/* true once mib registers have been cleared */
        void            *sbh;           /* sb utils handle */

        char            *vars;          /* sprom name=value */
        int             vars_size;

        void            *robo;          /* robo utils handle */
};

void chipphywr(ch_t *ch, uint phyaddr, uint reg, uint16 v);
uint16 chipphyrd(ch_t *ch, uint phyaddr, uint reg);


#endif	/* _et_priv_h_ */
