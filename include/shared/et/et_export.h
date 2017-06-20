/*
 * $Id: et_export.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Required functions exported by the port-specific (os-dependent) driver
 * to common (os-independent) driver code.
 */

#ifndef _et_export_h_
#define _et_export_h_

/* misc callbacks */
extern void et_soc_init(void *et, bool full);
extern void et_soc_reset(void *et);
extern void et_soc_link_up(void *et);
extern void et_soc_link_down(void *et);
extern int et_soc_up(void *et);
extern int et_soc_down(void *et, int reset);
extern void et_soc_dump(void *et, uchar *buf, uint len);

/* for BCM5222 dual-phy shared mdio contortion */
extern void *et_soc_phyfind(void *et, uint coreunit);
extern uint16 et_soc_phyrd(void *et, uint phyaddr, uint reg);
extern void et_soc_phywr(void *et, uint reg, uint phyaddr, uint16 val);

#endif	/* _et_export_h_ */
