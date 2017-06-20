/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * BCM47XX Sonics SiliconBackplane PCI core hardware definitions.
 *
 * $Id: sbpci.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 * Copyright(c) 2001 Broadcom Corporation
 */

#ifndef	_SBPCI_H
#define	_SBPCI_H

/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif

/* Sonics side: PCI core and host control registers */
typedef struct sbpciregs {
	uint32 control;		/* PCI control */
	uint32 PAD[3];
	uint32 arbcontrol;	/* PCI arbiter control */
	uint32 PAD[3];
	uint32 intstatus;	/* Interrupt status */
	uint32 intmask;		/* Interrupt mask */
	uint32 sbtopcimailbox;	/* Sonics to PCI mailbox */
	uint32 PAD[9];
	uint32 bcastaddr;	/* Sonics broadcast address */
	uint32 bcastdata;	/* Sonics broadcast data */
	uint32 PAD[2];
	uint32 gpioin;		/* ro: gpio input (>=rev2) */
	uint32 gpioout;		/* rw: gpio output (>=rev2) */
	uint32 gpioouten;	/* rw: gpio output enable (>= rev2) */
	uint32 gpiocontrol;	/* rw: gpio control (>= rev2) */
	uint32 PAD[36];
	uint32 sbtopci0;	/* Sonics to PCI translation 0 */
	uint32 sbtopci1;	/* Sonics to PCI translation 1 */
	uint32 sbtopci2;	/* Sonics to PCI translation 2 */
	uint32 PAD[445];
	uint16 sprom[36];	/* SPROM shadow Area */
	uint32 PAD[46];
} sbpciregs_t;

/* PCI control */
#define PCI_RST_OE	0x01	/* When set, drives PCI_RESET out to pin */
#define PCI_RST		0x02	/* Value driven out to pin */
#define PCI_CLK_OE	0x04	/* When set, drives clock as gated by PCI_CLK out to pin */
#define PCI_CLK		0x08	/* Gate for clock driven out to pin */	

/* PCI arbiter control */
#define PCI_INT_ARB	0x01	/* When set, use an internal arbiter */
#define PCI_EXT_ARB	0x02	/* When set, use an external arbiter */
#define PCI_PARKID_MASK	0x06	/* Selects which agent is parked on an idle bus */
#define PCI_PARKID_SHIFT   1
#define PCI_PARKID_LAST	   0	/* Last requestor */
#define PCI_PARKID_4710	   1	/* 4710 */
#define PCI_PARKID_EXTREQ0 2	/* External requestor 0 */
#define PCI_PARKID_EXTREQ1 3	/* External requestor 1 */

/* Interrupt status/mask */
#define PCI_INTA	0x01	/* PCI INTA# is asserted */
#define PCI_INTB	0x02	/* PCI INTB# is asserted */
#define PCI_SERR	0x04	/* PCI SERR# has been asserted (write one to clear) */
#define PCI_PERR	0x08	/* PCI PERR# has been asserted (write one to clear) */
#define PCI_PME		0x10	/* PCI PME# is asserted */

/* (General) PCI/SB mailbox interrupts, two bits per pci function */
#define	MAILBOX_F0_0	0x100	/* function 0, int 0 */
#define	MAILBOX_F0_1	0x200	/* function 0, int 1 */
#define	MAILBOX_F1_0	0x400	/* function 1, int 0 */
#define	MAILBOX_F1_1	0x800	/* function 1, int 1 */
#define	MAILBOX_F2_0	0x1000	/* function 2, int 0 */
#define	MAILBOX_F2_1	0x2000	/* function 2, int 1 */
#define	MAILBOX_F3_0	0x4000	/* function 3, int 0 */
#define	MAILBOX_F3_1	0x8000	/* function 3, int 1 */

/* Sonics broadcast address */
#define BCAST_ADDR_MASK	0xff	/* Broadcast register address */

/* Sonics to PCI translation types */
#define SBTOPCI0_MASK	0xfc000000
#define SBTOPCI1_MASK	0xfc000000
#define SBTOPCI2_MASK	0xc0000000
#define SBTOPCI_MEM	0
#define SBTOPCI_IO	1
#define SBTOPCI_CFG0	2
#define SBTOPCI_CFG1	3
#define	SBTOPCI_PREF	0x4	/* prefetch enable */
#define	SBTOPCI_BURST	0x8	/* burst enable */

/* PCI side: Reserved PCI configuration registers (see pcicfg.h) */
#define cap_list	rsvd_a[0]
#define bar0_window	dev_dep[0x80 - 0x40]
#define bar1_window	dev_dep[0x84 - 0x40]
#define sprom_control	dev_dep[0x88 - 0x40]

/* pci config registers */
#define	PCI_BAR0_WIN		0x80
#define	PCI_BAR1_WIN		0x84
#define	PCI_SPROM_CONTROL	0x88
#define	PCI_BAR1_CONTROL	0x8c
#define	PCI_INT_STATUS		0x90
#define	PCI_INT_MASK		0x94

#define	SBIM_SHIFT		8	/* pciintmask */
#define	SBIM_MASK		0xff00

#define	SPROM_BLANK		0x04  	/* sprom control bit indicating a blank sprom */
#define SPROM_WRITEEN		0x10	/* sprom write enable */

#define	SPROM_SIZE			256		/* sprom size in 16-bit */
#define SPROM_CRC_RANGE		64		/* crc cover range in 16-bit */

#define	PCI_GPIO_IN		0xb0	/* pci config space gpio input (>=rev3) */
#define	PCI_GPIO_OUT		0xb4	/* pci config space gpio output (>=rev3) */
#define	PCI_GPIO_OUTEN		0xb8	/* pci config space gpio output enable (>=rev3) */

#define	PCI_BAR0_SPROM_OFFSET	(4 * 1024)	/* bar0 + 4K accesses external sprom */
#define	PCI_BAR0_PCIREGS_OFFSET	(6 * 1024)	/* bar0 + 6K accesses pci core registers */

#endif	/* _SBPCI_H */
