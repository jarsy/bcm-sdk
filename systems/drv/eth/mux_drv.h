/*
 * $Id: mux_drv.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
 
#ifndef MUX_DRV_H
#define MUX_DRV_H

#define ETH_MAX_UNITS  4

typedef int (*cb_f)(int unit, void *b, int l, void *cookie);
typedef void * (*alloc_f)(int l);
typedef void (*free_f)(void *b);

extern int et_drvTxRxInit(char * dev, int unit);
extern int et_drvMIIRxHandlerRegister(int unit, cb_f cf, alloc_f af, free_f ff, void *cookie);
extern int et_drvMIIRxHandlerUnRegister(int unit, cb_f f);
extern int et_drvTx(int unit, unsigned char *b, int l, void *cookie);
extern int et_drv_mac_addr_get(int unit, unsigned char *mac);
extern void et_drv_start(int unit);
extern void et_drv_stop(int unit);

#endif /* MUX_DRV_H */

