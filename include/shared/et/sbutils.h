/*
 * $Id: sbutils.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Misc utility routines for accessing chip-specific features
 * of Broadcom HNBU SiliconBackplane-based chips.
 */

#ifndef	_sbutils_h_
#define	_sbutils_h_

/* Board styles (bustype) */
#define	BOARDSTYLE_SOC		0		/* Silicon Backplane */
#define	BOARDSTYLE_PCI		1		/* PCI/MiniPCI board */
#define	BOARDSTYLE_PCMCIA   	2		/* PCMCIA board */
#define	BOARDSTYLE_CARDBUS	3		/* Cardbus board */

#define DATA_BIT_16         1
#define DATA_BIT_8           2
/*
 * Many of the routines below take an 'sbh' handle as their first arg.
 * Allocate this by calling sb_attach().  Free it by calling sb_detach().
 * At any one time, the sbh is logically focused on one particular sb core
 * (the "current core").
 * Use sb_setcore() or sb_setcoreidx() to change the association to another core.
 */

/* exported externs */
extern void *sb_soc_attach(uint pcidev, void *osh, void *regs, uint bustype, char **vars, int *varsz);
extern void *sb_soc_kattach(void);
extern void sb_soc_detach(void *sbh);
extern uint sb_soc_chip(void *sbh);
extern uint sb_soc_chiprev(void *sbh);
extern uint sb_soc_boardvendor(void *sbh);
extern uint sb_soc_boardtype(void *sbh);
extern uint sb_soc_boardstyle(void *sbh);
extern uint sb_soc_bus(void *sbh);
extern uint sb_soc_corelist(void *sbh, uint coreid[]);
extern uint sb_soc_coreid(void *sbh);
#ifndef VXWORKS
extern uint sb_coreid(void *sbh);
extern bool sb_taclear(void *sbh);
extern uint sb_corerev(void *sbh);
extern void sb_core_disable(void *sbh, uint32 bits);
#endif
extern uint sb_coreidx(void *sbh);
extern uint sb_soc_coreunit(void *sbh);
extern uint sb_soc_corevendor(void *sbh);
extern uint sb_soc_corerev(void *sbh);
extern uint32 sb_soc_coreflags(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_soc_coreflagshi(void *sbh, uint32 mask, uint32 val);
extern bool sb_soc_iscoreup(void *sbh);
extern void *sb_setcoreidx(void *sbh, uint coreidx);
extern void *sb_soc_setcore(void *sbh, uint coreid, uint coreunit);
extern void sb_soc_commit(void *sbh);
extern uint32 sb_base(uint32 admatch);
extern uint32 sb_soc_size(uint32 admatch);
extern void sb_soc_core_reset(void *sbh, uint32 bits);
extern void sb_soc_core_tofixup(void *sbh);
extern void sb_soc_core_disable(void *sbh, uint32 bits);
extern uint32 sb_soc_clock_rate(uint32 pll_type, uint32 n, uint32 m);
extern uint32 sb_soc_clock(void *sbh);
extern bool sb_soc_setclock(void *sbh, uint32 sb, uint32 pci);
extern void sb_soc_pci_setup(void *sbh, uint32 *dmaoffset, uint coremask);
extern void sb_soc_pcmcia_init(void *sbh);
extern void sb_soc_chip_reset(void *sbh);
extern void *sb_gpiosetcore(void *sbh);
extern uint32 sb_soc_gpiocontrol(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_gpioouten(void *sbh, uint32 mask, uint32 val);
extern void sb_gpioout(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_gpioin(void *sbh);
extern uint32 sb_soc_gpiointpolarity(void *sbh, uint32 mask, uint32 val);
extern uint32 sb_soc_gpiointmask(void *sbh, uint32 mask, uint32 val);
extern void sb_soc_dump(void *sbh, char *buf);
extern bool sb_soc_taclear(void *sbh);
extern uint32 sb_EBbus_enable(void * sbh, int data_type);

/* nvram related */
extern char *getvar(char *vars, char *name);
extern int getintvar(char *vars, char *name);
extern int sromread(void *sbh, uint byteoff, uint nbytes, uint16 *buf);
extern int sromwrite(void *sbh, uint byteoff, uint nbytes, uint16 *buf);
extern int parsecis(uint8 *cis, char **vars, int *count);

#endif	/* _sbutils_h_ */
