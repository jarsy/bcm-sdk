/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Required functions exported by the port-specific (os-dependent) driver
 * to common (os-independent) driver code.
 *
 * Copyright(c) 2000 Broadcom, Inc.
 * $Id: et_export.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _et_export_h_
#define _et_export_h_

/* misc callbacks */
extern void et_init(void *et);
extern void et_reset(void *et);
extern void et_link_up(void *et);
extern void et_link_down(void *et);
extern int et_up(void *et);
extern int et_down(void *et, int reset);
extern void et_dump(void *et, uchar *buf, uint len);

#ifdef BCM47XX_CHOPS
/* for BCM5222 dual-phy shared mdio contortion */
extern void *et_phyfind(void *et, uint coreunit);
extern uint16 et_phyrd(void *et, uint phyaddr, uint reg);
extern void et_phywr(void *et, uint reg, uint phyaddr, uint16 val);
#endif

#endif	/* _et_export_h_ */
