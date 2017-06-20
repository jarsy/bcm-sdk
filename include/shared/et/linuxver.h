/*
 * $Id: linuxver.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Linux-specific abstractions to gain some independence from different linux kernel versions.
 */

#ifndef _linuxver_h_
#define _linuxver_h_

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0))
#include <linux/kconfig.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
#include <linux/config.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
/* __NO_VERSION__ must be defined for all linkables except one in 2.2 */
#ifdef __UNDEF_NO_VERSION__
#undef __NO_VERSION__
#else
#define __NO_VERSION__
#endif
#endif

#if defined(MODULE) && defined(MODVERSIONS)
#include <linux/modversions.h>
#endif

/* linux/malloc.h is deprecated, use linux/slab.h instead. */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,9))
#include <linux/malloc.h>
#else
#include <linux/slab.h>
#endif

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <asm/io.h>

#ifndef __exit
#define __exit
#endif
#ifndef __devexit
#define __devexit
#endif
#ifndef __devinit
#define __devinit	__init
#endif
#ifndef __devinitdata
#define __devinitdata
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))

#define pci_get_drvdata(dev)		(dev)->sysdata
#define pci_set_drvdata(dev, value)	(dev)->sysdata=(value)

/*
 * New-style (2.4.x) PCI/hot-pluggable PCI/CardBus registration
 */

struct pci_device_id {
	unsigned int vendor;		/* Vendor and device ID or PCI_ANY_ID */
	unsigned int device;
	unsigned int subvendor;	/* Subsystem ID's or PCI_ANY_ID */
	unsigned int subdevice;
	unsigned int class;		/* (class,subclass,prog-if) triplet */
	unsigned int class_mask;
	unsigned long driver_data;		/* Data private to the driver */
};

tpedef struct pci_driver {
	struct list_head node;
	char *name;
	const struct pci_device_id *id_table;	/* NULL if wants all devices */
	int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);	/* New device inserted */
	void (*remove)(struct pci_dev *dev);	/* Device removed (NULL if not a hot-plug capable driver) */
	void (*suspend)(struct pci_dev *dev);	/* Device suspended */
	void (*resume)(struct pci_dev *dev);	/* Device woken up */
} pci_driver_t;

#define MODULE_DEVICE_TABLE(type, name)
#define PCI_ANY_ID (~0)

/* compatpci.c */
#define pci_module_init pci_register_driver
extern int pci_register_driver(pci_driver_t *drv);
extern void pci_unregister_driver(pci_driver_t *drv);

#endif /* PCI registration */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18))
#ifdef MODULE
#define module_init(x) int init_module(void) { return x(); }
#define module_exit(x) void cleanup_module(void) { x(); }
#else
#define module_init(x)	__initcall(x);
#define module_exit(x)	__exitcall(x);
#endif
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,48))
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,13))
#define pci_resource_start(dev, bar)	((dev)->base_address[(bar)])
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,44))
#define pci_resource_start(dev, bar)	((dev)->resource[(bar)].start)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,23))
#define pci_enable_device(dev) do { } while (0)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,14))
#define net_device device
#endif


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,43))

#define dev_kfree_skb_any(a)		dev_kfree_skb(a)
#define netif_down(dev)			do { (dev)->start = 0; } while(0)

/* pcmcia-cs provides its own netdevice compatibility layer */
#ifndef _COMPAT_NETDEVICE_H

/*
 * SoftNet
 *
 * For pre-softnet kernels we need to tell the upper layer not to
 * re-enter start_xmit() while we are in there. However softnet
 * guarantees not to enter while we are in there so there is no need
 * to do the netif_stop_queue() dance unless the transmit queue really
 * gets stuck. This should also improve performance according to tests
 * done by Aman Singla.
 */

#define dev_kfree_skb_irq(a)		dev_kfree_skb(a)
#define netif_wake_queue(dev)		do { clear_bit(0, &(dev)->tbusy); mark_bh(NET_BH); } while(0)
#define netif_stop_queue(dev)		set_bit(0, &(dev)->tbusy)

static inline void netif_start_queue(struct net_device *dev)
{
	dev->tbusy = 0;
	dev->interrupt = 0;
	dev->start = 1;
}

#define netif_queue_stopped(dev)	(dev)->tbusy
#define netif_running(dev)		(dev)->start

#endif /* _COMPAT_NETDEVICE_H */

#define netif_device_attach(dev)	netif_start_queue(dev)
#define netif_device_detach(dev)	netif_stop_queue(dev)

/* 2.4.x renamed bottom halves to tasklets */
#define tasklet_struct				tq_struct
static inline void tasklet_schedule(struct tasklet_struct *tasklet)
{
	queue_task(tasklet, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

static inline void tasklet_init(struct tasklet_struct *tasklet,
				void (*func)(unsigned long),
				unsigned long data)
{
	tasklet->next = NULL;
	tasklet->sync = 0;
	tasklet->routine = (void (*)(void *))func;
	tasklet->data = (void *)data;
}
#define tasklet_kill(tasklet)			{do{} while(0);}

/* 2.4.x introduced del_timer_sync() */
#define del_timer_sync(timer) del_timer(timer)

#else

#define netif_down(dev)

#endif /* SoftNet */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,3))

/*
 * Emit code to initialise a tq_struct's routine and data pointers
 */
#define PREPARE_TQUEUE(_tq, _routine, _data)			\
	do {							\
		(_tq)->routine = _routine;			\
		(_tq)->data = _data;				\
	} while (0)

/*
 * Emit code to initialise all of a tq_struct
 */
#define INIT_TQUEUE(_tq, _routine, _data)			\
	do {							\
		INIT_LIST_HEAD(&(_tq)->list);			\
		(_tq)->sync = 0;				\
		PREPARE_TQUEUE((_tq), (_routine), (_data));	\
	} while (0)

#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,6))

/* Power management related routines */

static inline int
pci_save_state(struct pci_dev *dev, u32 *buffer)
{
	int i;
	if (buffer) {
		/* XXX: 100% dword access ok here? */
		for (i = 0; i < 16; i++)
			pci_read_config_dword(dev, i * 4,&buffer[i]);
	}
	return 0;
}

static inline int 
pci_restore_state(struct pci_dev *dev, u32 *buffer)
{
	int i;

	if (buffer) {
		for (i = 0; i < 16; i++)
			pci_write_config_dword(dev,i * 4, buffer[i]);
	}
	/*
	 * otherwise, write the context information we know from bootup.
	 * This works around a problem where warm-booting from Windows
	 * combined with a D3(hot)->D0 transition causes PCI config
	 * header data to be forgotten.
	 */	
	else {
		for (i = 0; i < 6; i ++)
			pci_write_config_dword(dev,
					       PCI_BASE_ADDRESS_0 + (i * 4),
					       pci_resource_start(dev, i));
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
	}
	return 0;
}

#endif /* PCI power management */

/* Old cp0 access macros deprecated in 2.4.19 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,19))
#define read_c0_count() read_32bit_cp0_register(CP0_COUNT)
#endif

#endif /* _linuxver_h_ */
