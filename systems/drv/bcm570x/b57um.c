/*
 * $Id: b57um.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/******************************************************************************/
/*                                                                            */
/* Broadcom BCM5700 Linux Network Driver, Copyright (c) 2001 Broadcom         */
/* Corporation.                                                               */
/* All rights reserved.                                                       */
/*                                                                            */
/******************************************************************************/


char bcm5700_driver[] = "bcm5700";
char bcm5700_version[] = "3.0.2";
char bcm5700_date[] = "(05/03/02)";

#define B57UM
#include "mm.h"

#define TASKLET

/* A few user-configurable values. */

#define MAX_UNITS 16
/* Used to pass the full-duplex flag, etc. */
static int line_speed[MAX_UNITS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int auto_speed[MAX_UNITS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static int full_duplex[MAX_UNITS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static int rx_flow_control[MAX_UNITS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int tx_flow_control[MAX_UNITS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int auto_flow_control[MAX_UNITS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int mtu[MAX_UNITS] = {1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500};	/* Jumbo MTU for interfaces. */
static int tx_checksum[MAX_UNITS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static int rx_checksum[MAX_UNITS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static int scatter_gather[MAX_UNITS] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

#define TX_DESC_CNT DEFAULT_TX_PACKET_DESC_COUNT
static unsigned int tx_pkt_desc_cnt[MAX_UNITS] =
	{TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,
	TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,
	TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT,
	TX_DESC_CNT};

#define RX_DESC_CNT DEFAULT_STD_RCV_DESC_COUNT
static unsigned int rx_std_desc_cnt[MAX_UNITS] =
	{RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,
	RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,
	RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,
	RX_DESC_CNT };

#define JBO_DESC_CNT DEFAULT_JUMBO_RCV_DESC_COUNT
static unsigned int rx_jumbo_desc_cnt[MAX_UNITS] =
	{JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,
	JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,
	JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,JBO_DESC_CNT,
	JBO_DESC_CNT };

static unsigned int adaptive_coalesce[MAX_UNITS] =
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

#define RX_COAL_TK DEFAULT_RX_COALESCING_TICKS
static unsigned int rx_coalesce_ticks[MAX_UNITS] =
	{RX_COAL_TK,RX_COAL_TK,RX_COAL_TK,RX_COAL_TK,RX_COAL_TK,
	RX_COAL_TK, RX_COAL_TK,RX_COAL_TK,RX_COAL_TK,RX_COAL_TK,
	RX_COAL_TK,RX_COAL_TK, RX_COAL_TK,RX_COAL_TK,RX_COAL_TK,
	RX_COAL_TK};

#define RX_COAL_FM DEFAULT_RX_MAX_COALESCED_FRAMES
static unsigned int rx_max_coalesce_frames[MAX_UNITS] =
	{RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,
	RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,
	RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,RX_COAL_FM,
	RX_COAL_FM};

#define TX_COAL_TK DEFAULT_TX_COALESCING_TICKS
static unsigned int tx_coalesce_ticks[MAX_UNITS] =
	{TX_COAL_TK,TX_COAL_TK,TX_COAL_TK,TX_COAL_TK,TX_COAL_TK,
	TX_COAL_TK, TX_COAL_TK,TX_COAL_TK,TX_COAL_TK,TX_COAL_TK,
	TX_COAL_TK,TX_COAL_TK, TX_COAL_TK,TX_COAL_TK,TX_COAL_TK,
	TX_COAL_TK};

#define TX_COAL_FM DEFAULT_TX_MAX_COALESCED_FRAMES
static unsigned int tx_max_coalesce_frames[MAX_UNITS] =
	{TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,
	TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,
	TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,TX_COAL_FM,
	TX_COAL_FM};

#define ST_COAL_TK DEFAULT_STATS_COALESCING_TICKS
static unsigned int stats_coalesce_ticks[MAX_UNITS] =
	{ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,
	ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,
	ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,ST_COAL_TK,
	ST_COAL_TK,};

static int enable_wol[MAX_UNITS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* Operational parameters that usually are not changed. */
/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT  (2*HZ)

#if (LINUX_VERSION_CODE < 0x02030d)
#define pci_resource_start(dev, bar)	(dev->base_address[bar] & PCI_BASE_ADDRESS_MEM_MASK)
#elif (LINUX_VERSION_CODE < 0x02032b)
#define pci_resource_start(dev, bar)	(dev->resource[bar] & PCI_BASE_ADDRESS_MEM_MASK)
#endif

#if (LINUX_VERSION_CODE < 0x02032b)
#define dev_kfree_skb_irq(skb)  dev_kfree_skb(skb)
#define netif_wake_queue(dev)	clear_bit(0, &dev->tbusy); mark_bh(NET_BH)
#define netif_stop_queue(dev)	set_bit(0, &dev->tbusy)

static inline void netif_start_queue(struct net_device *dev)
{
	dev->tbusy = 0;
	dev->interrupt = 0;
	dev->start = 1;
}

#define netif_queue_stopped(dev)	dev->tbusy
#define netif_running(dev)		dev->start

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

#define tasklet_kill(tasklet)

#endif

#if (LINUX_VERSION_CODE < 0x020300)
struct pci_device_id {
	unsigned int vendor, device;		/* Vendor and device ID or PCI_ANY_ID */
	unsigned int subvendor, subdevice;	/* Subsystem ID's or PCI_ANY_ID */
	unsigned int class, class_mask;		/* (class,subclass,prog-if) triplet */
	unsigned long driver_data;		/* Data private to the driver */
};

#define PCI_ANY_ID		0

#define pci_set_drvdata(pdev, dev)
#define pci_get_drvdata(pdev) 0

#define pci_enable_device(pdev) 0

#define __devinit		__init
#define __devinitdata		__initdata
#define __devexit

#define SET_MODULE_OWNER(dev)
#define MODULE_DEVICE_TABLE(pci, pci_tbl)

#endif

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(license)
#endif

#if (LINUX_VERSION_CODE < 0x02032a)
static inline void *pci_alloc_consistent(struct pci_dev *pdev, size_t size,
					 dma_addr_t *dma_handle)
{
	void *virt_ptr;

	/* Maximum in slab.c */
	if (size > 131072)
		return 0;

	virt_ptr = kmalloc(size, GFP_KERNEL);
	*dma_handle = virt_to_bus(virt_ptr);
	return virt_ptr;
}
#define pci_free_consistent(dev, size, ptr, dma_ptr)	kfree(ptr)

#endif /*#if (LINUX_VERSION_CODE < 0x02032a) */


#if (LINUX_VERSION_CODE < 0x020329)
#define pci_set_dma_mask(pdev, mask) (0)
#else
#if (LINUX_VERSION_CODE < 0x020403)
int
pci_set_dma_mask(struct pci_dev *dev, dma_addr_t mask)
{
    if(! pci_dma_supported(dev, mask))
        return -EIO;

    dev->dma_mask = mask;

    return 0;
}
#endif
#endif

#if (LINUX_VERSION_CODE < 0x020402)
#define pci_request_regions(pdev, name) (0)
#define pci_release_regions(pdev)
#endif

#if ! defined(spin_is_locked)
#define spin_is_locked(lock)    (test_bit(0,(lock)))
#endif

inline long
bcm5700_lock(PUM_DEVICE_BLOCK pUmDevice)
{
	long flags;
	
	if (pUmDevice->do_global_lock) {
		spin_lock_irqsave(&pUmDevice->global_lock, flags);
		return flags;
	}
	return 0;
}

inline void
bcm5700_unlock(PUM_DEVICE_BLOCK pUmDevice, long flags)
{
	if (pUmDevice->do_global_lock) {
		spin_unlock_irqrestore(&pUmDevice->global_lock, flags);
	}
}

inline int
bcm5700_trylock(PUM_DEVICE_BLOCK pUmDevice, long *flags)
{
	if (pUmDevice->do_global_lock) {
		if (spin_is_locked(&pUmDevice->global_lock))
			return 0;
		spin_lock_irqsave(&pUmDevice->global_lock, *flags);
		return 1;
	}
	return 1;
}

inline void
bcm5700_intr_lock(PUM_DEVICE_BLOCK pUmDevice)
{
	if (pUmDevice->do_global_lock) {
		spin_lock(&pUmDevice->global_lock);
	}
}

inline void
bcm5700_intr_unlock(PUM_DEVICE_BLOCK pUmDevice)
{
	if (pUmDevice->do_global_lock) {
		spin_unlock(&pUmDevice->global_lock);
	}
}

/*
 * Broadcom NIC Extension support
 * -ffan
 */
#ifdef NICE_SUPPORT
#include "nicext.h"

typedef struct {
	ushort  tag;
	ushort  signature;
} vlan_tag_t;

#endif /* NICE_SUPPORT */

int MM_Packet_Desc_Size = sizeof(UM_PACKET);

#define RUN_AT(x) (jiffies + (x))

char kernel_version[] = UTS_RELEASE;

#define PCI_SUPPORT_VER2

#if ! defined(CAP_NET_ADMIN)
#define capable(CAP_XXX) (suser())
#endif

#define tigon3_debug debug
#if TIGON3_DEBUG
static int tigon3_debug = TIGON3_DEBUG;
#else
static int tigon3_debug = 0;
#endif


STATIC int bcm5700_open(struct net_device *dev);
STATIC void bcm5700_timer(unsigned long data);
STATIC void bcm5700_tx_timeout(struct net_device *dev);
STATIC int bcm5700_start_xmit(struct sk_buff *skb, struct net_device *dev);
STATIC void bcm5700_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
STATIC void bcm5700_tasklet(unsigned long data);
STATIC int bcm5700_close(struct net_device *dev);
STATIC struct net_device_stats *bcm5700_get_stats(struct net_device *dev);
STATIC int bcm5700_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
STATIC void bcm5700_set_rx_mode(struct net_device *dev);
STATIC int bcm5700_set_mac_addr(struct net_device *dev, void *p);
STATIC int replenish_rx_buffers(PUM_DEVICE_BLOCK pUmDevice);
STATIC int bcm5700_freemem(struct net_device *dev);
STATIC int bcm5700_adapt_coalesce(PUM_DEVICE_BLOCK pUmDevice);


/* A list of all installed bcm5700 devices. */
static struct net_device *root_tigon3_dev = NULL;

typedef enum {
	BCM5700VIGIL = 0,
	BCM5700A6,
	BCM5700T6,
	BCM5700A9,
	BCM5700T9,
	BCM5700,
	BCM5701A5,
	BCM5701T1,
	BCM5701T8,
	BCM5701A7,
	BCM5701A10,
	BCM5701A12,
	BCM5701,
	BCM5702,
	BCM5703,
	BCM5703A31,
	TC996T,
	TC996ST,
	TC996SSX,
	TC996SX,
	TC996BT,
	TC997T,
	TC997SX,
	TC1000T,
	TC940BR01,
	TC942BR01,
	NC6770,
	NC7760,
	NC7770,
	NC7771,
	NC7780,
	NC7781,
	BCM5704,
} board_t;


/* indexed by board_t, above */
static struct {
	char *name;
} board_info[] __devinitdata = {
	{ "Broadcom Vigil B5700 1000Base-T" },
	{ "Broadcom BCM5700 1000Base-T" },
	{ "Broadcom BCM5700 1000Base-SX" },
	{ "Broadcom BCM5700 1000Base-SX" },
	{ "Broadcom BCM5700 1000Base-T" },
	{ "Broadcom BCM5700" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-SX" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701" },
	{ "Broadcom BCM5702 1000Base-T" },
	{ "Broadcom BCM5703 1000Base-T" },
	{ "Broadcom BCM5703 1000Base-SX" },
	{ "3Com 3C996 10/100/1000 Server NIC" },
	{ "3Com 3C996 10/100/1000 Server NIC" },
	{ "3Com 3C996 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C996 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C996B Gigabit Server NIC" },
	{ "3Com 3C997 Gigabit Server NIC" },
	{ "3Com 3C997 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C1000 Gigabit NIC" },
	{ "3Com 3C940 Gigabit LOM (21X21)" },
	{ "3Com 3C942 Gigabit LOM (31X31)" },
	{ "Compaq NC6770 Gigabit Server Adapter" },
	{ "Compaq NC7760 Gigabit Server Adapter" },
	{ "Compaq NC7770 Gigabit Server Adapter" },
	{ "Compaq NC7771 Gigabit Server Adapter" },
	{ "Compaq NC7780 Gigabit Server Adapter" },
	{ "Compaq NC7781 Gigabit Server Adapter" },
	{ "Broadcom BCM5704 1000Base-T" },
	{ 0 },
	};

static struct pci_device_id bcm5700_pci_tbl[] __devinitdata = {
	{0x14e4, 0x1644, 0x1014, 0x0277, 0, 0, BCM5700VIGIL },
	{0x14e4, 0x1644, 0x14e4, 0x1644, 0, 0, BCM5700A6 },
	{0x14e4, 0x1644, 0x14e4, 0x2, 0, 0, BCM5700T6 },
	{0x14e4, 0x1644, 0x14e4, 0x3, 0, 0, BCM5700A9 },
	{0x14e4, 0x1644, 0x14e4, 0x4, 0, 0, BCM5700T9 },
	{0x14e4, 0x1644, 0x1028, 0xd1, 0, 0, BCM5700 },
	{0x14e4, 0x1644, 0x1028, 0x0106, 0, 0, BCM5700 },
	{0x14e4, 0x1644, 0x1028, 0x0109, 0, 0, BCM5700 },
	{0x14e4, 0x1644, 0x1028, 0x010a, 0, 0, BCM5700 },
	{0x14e4, 0x1644, 0x10b7, 0x1000, 0, 0, TC996T },
	{0x14e4, 0x1644, 0x10b7, 0x1001, 0, 0, TC996ST },
	{0x14e4, 0x1644, 0x10b7, 0x1002, 0, 0, TC996SSX },
	{0x14e4, 0x1644, 0x10b7, 0x1003, 0, 0, TC997T },
	{0x14e4, 0x1644, 0x10b7, 0x1005, 0, 0, TC997SX },
	{0x14e4, 0x1644, 0x10b7, 0x1008, 0, 0, TC942BR01 },
	{0x14e4, 0x1644, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5700 },
	{0x14e4, 0x1645, 0x14e4, 1, 0, 0, BCM5701A5 },
	{0x14e4, 0x1645, 0x14e4, 5, 0, 0, BCM5701T1 },
	{0x14e4, 0x1645, 0x14e4, 6, 0, 0, BCM5701T8 },
	{0x14e4, 0x1645, 0x14e4, 7, 0, 0, BCM5701A7 },
	{0x14e4, 0x1645, 0x14e4, 8, 0, 0, BCM5701A10 },
	{0x14e4, 0x1645, 0x14e4, 0x8008, 0, 0, BCM5701A12 },
	{0x14e4, 0x1645, 0x0e11, 0xc1, 0, 0, NC6770 },
	{0x14e4, 0x1645, 0x0e11, 0x7c, 0, 0, NC7770 },
	{0x14e4, 0x1645, 0x0e11, 0x85, 0, 0, NC7780 },
	{0x14e4, 0x1645, 0x1028, 0x0121, 0, 0, BCM5701 },
	{0x14e4, 0x1645, 0x10b7, 0x1004, 0, 0, TC996SX },
	{0x14e4, 0x1645, 0x10b7, 0x1006, 0, 0, TC996BT },
	{0x14e4, 0x1645, 0x10b7, 0x1007, 0, 0, TC1000T },
	{0x14e4, 0x1645, 0x10b7, 0x1008, 0, 0, TC940BR01 },
	{0x14e4, 0x1645, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5701 },
	{0x14e4, 0x1646, 0x14e4, 0x8009, 0, 0, BCM5702 },
	{0x14e4, 0x1646, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5702 },
	{0x14e4, 0x16a6, 0x14e4, 0x8009, 0, 0, BCM5702 },
	{0x14e4, 0x16a6, 0x14e4, 0x000c, 0, 0, BCM5702 },
	{0x14e4, 0x16a6, 0x0e11, 0xbb, 0, 0, NC7760 },
	{0x14e4, 0x16a6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5702 },
	{0x14e4, 0x1647, 0x14e4, 0x0009, 0, 0, BCM5703 },
	{0x14e4, 0x1647, 0x14e4, 0x000a, 0, 0, BCM5703A31 },
	{0x14e4, 0x1647, 0x14e4, 0x000b, 0, 0, BCM5703 },
	{0x14e4, 0x1647, 0x14e4, 0x800a, 0, 0, BCM5703 },
	{0x14e4, 0x1647, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5703 },
	{0x14e4, 0x16a7, 0x14e4, 0x0009, 0, 0, BCM5703 },
	{0x14e4, 0x16a7, 0x14e4, 0x000a, 0, 0, BCM5703A31 },
	{0x14e4, 0x16a7, 0x14e4, 0x000b, 0, 0, BCM5703 },
	{0x14e4, 0x16a7, 0x14e4, 0x800a, 0, 0, BCM5703 },
	{0x14e4, 0x16a7, 0x0e11, 0xca, 0, 0, NC7771 },
	{0x14e4, 0x16a7, 0x0e11, 0xcb, 0, 0, NC7781 },
	{0x14e4, 0x16a7, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5703 },
	{0x14e4, 0x1648, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5704 },
	{0,}
};

MODULE_DEVICE_TABLE(pci, bcm5700_pci_tbl);

#ifdef CONFIG_PROC_FS
extern int bcm5700_proc_create(void);
extern int bcm5700_proc_create_dev(struct net_device *dev);
extern int bcm5700_proc_remove_dev(struct net_device *dev);
#endif

static int __devinit bcm5700_init_board(struct pci_dev *pdev,
					struct net_device **dev_out,
					int board_idx)
{
	struct net_device *dev;
	PUM_DEVICE_BLOCK pUmDevice;
	PLM_DEVICE_BLOCK pDevice;
	int rc;

	*dev_out = NULL;

	/* dev zeroed in init_etherdev */
	dev = init_etherdev(NULL, sizeof(*pUmDevice));
	if (dev == NULL) {
		printk (KERN_ERR "%s: unable to alloc new ethernet\n",
			bcm5700_driver);
		return -ENOMEM;
	}
	SET_MODULE_OWNER(dev);
	pUmDevice = (PUM_DEVICE_BLOCK) dev->priv;

	/* enable device (incl. PCI PM wakeup), and bus-mastering */
	rc = pci_enable_device (pdev);
	if (rc)
		goto err_out;

	rc = pci_request_regions(pdev, bcm5700_driver);
	if (rc)
		goto err_out;

	pci_set_master(pdev);

	if (pci_set_dma_mask(pdev, ~(0UL)) != 0) {
		printk(KERN_ERR "System does not support DMA\n");
		pci_release_regions(pdev);
		goto err_out;
	}

	pUmDevice->dev = dev;
	pUmDevice->pdev = pdev;
	pUmDevice->mem_list_num = 0;
	pUmDevice->next_module = root_tigon3_dev;
	pUmDevice->index = board_idx;
	root_tigon3_dev = dev;

	spin_lock_init(&pUmDevice->global_lock);

	spin_lock_init(&pUmDevice->undi_lock);


	pDevice = (PLM_DEVICE_BLOCK) pUmDevice;

	if (mtu[board_idx] > 1500) {
		if (mtu[board_idx] > 9000) {
			dev->mtu = 9000;
			printk(KERN_WARNING "%s: Invalid mtu parameter (%d), using 9000\n", dev->name, mtu[board_idx]);
		}
		else
			dev->mtu = mtu[board_idx];
	}
	else if (mtu[board_idx] < 1500) {
		printk(KERN_WARNING "%s: Invalid mtu parameter (%d), using 1500\n", dev->name, mtu[board_idx]);
	}

	if (pci_find_device(0x8086, 0x2418, NULL) ||
		pci_find_device(0x8086, 0x2428, NULL)) {

		/* Found ICH or ICH0 */
		pDevice->UndiFix = 1;
	}

	if (LM_GetAdapterInfo(pDevice) != LM_STATUS_SUCCESS) {
		printk(KERN_ERR "Get Adapter info failed\n");
		rc = -ENODEV;
		goto err_out_unmap;
	}

	pUmDevice->do_global_lock = 0;
	if (T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5700) {
		/* The 5700 chip works best without interleaved register */
		/* accesses on certain machines. */
		pUmDevice->do_global_lock = 1;
	}
	if ((T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5701) &&
		((pDevice->PciState & T3_PCI_STATE_NOT_PCI_X_BUS) == 0)) {

		pUmDevice->rx_buf_align = 0;
	}
	else {
		pUmDevice->rx_buf_align = 2;
	}
/*	dev->base_addr = pci_resource_start(pdev, 0);*/
	dev->mem_start = pci_resource_start(pdev, 0);
	dev->mem_end = dev->mem_start + sizeof(T3_STD_MEM_MAP); 
	dev->irq = pDevice->Irq = pdev->irq;

	*dev_out = dev;
	return 0;

err_out_unmap:
	pci_release_regions(pdev);
	bcm5700_freemem(dev);

err_out:
	unregister_netdev(dev);
	kfree (dev);
	return rc;
}

static int __devinit
bcm5700_print_ver(void)
{
	printk(KERN_INFO "Broadcom Gigabit Ethernet Driver %s ",
		bcm5700_driver);
#ifdef NICE_SUPPORT
	printk("with Broadcom NIC Extension (NICE) ");
#endif
	printk("ver. %s %s\n", bcm5700_version, bcm5700_date);
	return 0;
}

static int __devinit
bcm5700_init_one(struct pci_dev *pdev,
				       const struct pci_device_id *ent)
{
	struct net_device *dev = NULL;
	PUM_DEVICE_BLOCK pUmDevice;
	PLM_DEVICE_BLOCK pDevice;
	int i;
	static int board_idx = -1;
	static int printed_version = 0;
	struct pci_dev *amd_dev;

	board_idx++;

	if (!printed_version) {
		bcm5700_print_ver();
#ifdef CONFIG_PROC_FS
		bcm5700_proc_create();
#endif
		printed_version = 1;
	}

	i = bcm5700_init_board(pdev, &dev, board_idx);
	if (i < 0) {
		return i;
	}

	if (dev == NULL)
		return -ENOMEM;

	dev->open = bcm5700_open;
	dev->hard_start_xmit = bcm5700_start_xmit;
	dev->stop = bcm5700_close;
	dev->get_stats = bcm5700_get_stats;
	dev->set_multicast_list = bcm5700_set_rx_mode;
	dev->do_ioctl = bcm5700_ioctl;
	dev->set_mac_address = &bcm5700_set_mac_addr;
#if (LINUX_VERSION_CODE >= 0x20400)
	dev->tx_timeout = bcm5700_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
#endif

	pUmDevice = (PUM_DEVICE_BLOCK) dev->priv;
	pDevice = (PLM_DEVICE_BLOCK) pUmDevice;

	dev->base_addr = pci_resource_start(pdev, 0);
	dev->irq = pdev->irq;

	pci_set_drvdata(pdev, dev);

	memcpy(dev->dev_addr, pDevice->NodeAddress, 6);
	pUmDevice->name = board_info[ent->driver_data].name,
	printk(KERN_INFO "%s: %s found at mem %lx, IRQ %d, ",
		dev->name, pUmDevice->name, dev->base_addr,
		dev->irq);
	printk("node addr ");
	for (i = 0; i < 6; i++) {
		printk("%2.2x", dev->dev_addr[i]);
	}
	printk("\n");

	printk(KERN_INFO "%s: ", dev->name);
	if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5400_PHY_ID)
		printk("Broadcom BCM5400 Copper ");
	else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5401_PHY_ID)
		printk("Broadcom BCM5401 Copper ");
	else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5411_PHY_ID)
		printk("Broadcom BCM5411 Copper ");
	else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5701_PHY_ID)
		printk("Broadcom BCM5701 Integrated Copper ");
	else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5703_PHY_ID)
		printk("Broadcom BCM5703 Integrated Copper ");
	else if ((pDevice->PhyId & PHY_ID_MASK) == PHY_BCM8002_PHY_ID)
		printk("Broadcom BCM8002 SerDes ");
	else if (pDevice->EnableTbi)
		printk("Agilent HDMP-1636 SerDes ");
	else
		printk("Unknown ");
	printk("transceiver found\n");

	printk(KERN_INFO "%s: ", dev->name);
#if (LINUX_VERSION_CODE >= 0x20400)
	if (scatter_gather[board_idx]) {
		dev->features |= NETIF_F_SG | NETIF_F_HIGHDMA;
	}
	if ((pDevice->ChipRevId != T3_CHIP_ID_5700_B0) &&
		tx_checksum[board_idx]) {
		dev->features |= NETIF_F_IP_CSUM;
	}

	printk("Scatter-gather %s, 64-bit DMA %s, Tx Checksum %s, ",
		(char *) ((dev->features & NETIF_F_SG) ? "ON" : "OFF"),
		(char *) ((dev->features & NETIF_F_HIGHDMA) ? "ON" : "OFF"),
		(char *) ((dev->features & NETIF_F_IP_CSUM) ? "ON" : "OFF"));
#endif
	if ((pDevice->ChipRevId != T3_CHIP_ID_5700_B0) &&
		rx_checksum[board_idx])
		printk("Rx Checksum ON\n");
	else
		printk("Rx Checksum OFF\n");

#ifdef CONFIG_PROC_FS
	bcm5700_proc_create_dev(dev);
#endif
#ifdef TASKLET
	tasklet_init(&pUmDevice->tasklet, bcm5700_tasklet,
		(unsigned long) pUmDevice);
#endif
	if ((amd_dev = pci_find_device(0x1022, 0x700c, NULL))) {
		u32 val;

		/* Found AMD 762 North bridge */
		pci_read_config_dword(amd_dev, 0x4c, &val);
		if ((val & 0x02) == 0) {
			pci_write_config_dword(amd_dev, 0x4c, val | 0x02);
			printk(KERN_INFO "%s: Setting AMD762 Northbridge to enable PCI ordering compliance\n", bcm5700_driver);
		}
	}
	return 0;

}


static void __devexit
bcm5700_remove_one (struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata (pdev);
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;

#ifdef CONFIG_PROC_FS
	bcm5700_proc_remove_dev(dev); 
#endif
	unregister_netdev(dev);

	if (pUmDevice->lm_dev.pMappedMemBase)
		iounmap(pUmDevice->lm_dev.pMappedMemBase);

	pci_release_regions(pdev);

	kfree(dev);

	pci_set_drvdata(pdev, NULL);

/*	pci_power_off(pdev, -1);*/

}

int __devinit
bcm5700_probe(struct net_device *dev)
{
	int cards_found = 0;
	struct pci_dev *pdev = NULL;
	struct pci_device_id *pci_tbl;
	u16 ssvid, ssid;

	if ( ! pci_present())
		return -ENODEV;

	pci_tbl = bcm5700_pci_tbl;
	while ((pdev = pci_find_class(PCI_CLASS_NETWORK_ETHERNET << 8, pdev))) {
		int idx;

		pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &ssvid);
		pci_read_config_word(pdev, PCI_SUBSYSTEM_ID, &ssid);
		for (idx = 0; pci_tbl[idx].vendor; idx++) {
			if ((pci_tbl[idx].vendor == PCI_ANY_ID ||
				pci_tbl[idx].vendor == pdev->vendor) &&
				(pci_tbl[idx].device == PCI_ANY_ID ||
				pci_tbl[idx].device == pdev->device) &&
				(pci_tbl[idx].subvendor == PCI_ANY_ID ||
				pci_tbl[idx].subvendor == ssvid) &&
				(pci_tbl[idx].subdevice == PCI_ANY_ID ||
				pci_tbl[idx].subdevice == ssid))
			{

				break;
			}
		}
		if (pci_tbl[idx].vendor == 0)
			continue;


		if (bcm5700_init_one(pdev, &pci_tbl[idx]) == 0)
			cards_found++;
	}

	return cards_found ? 0 : -ENODEV;
}



STATIC int
bcm5700_open(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	int index;

	index = pUmDevice->index;
	if (pDevice->ChipRevId == T3_CHIP_ID_5700_B0) {
		pDevice->TaskToOffload = LM_TASK_OFFLOAD_NONE; 
	}
	else {
		if (rx_checksum[index]) {
			pDevice->TaskToOffload |=
				LM_TASK_OFFLOAD_RX_TCP_CHECKSUM |
				LM_TASK_OFFLOAD_RX_UDP_CHECKSUM;
		}
		if (tx_checksum[index]) {
			pDevice->TaskToOffload |=
				LM_TASK_OFFLOAD_TX_TCP_CHECKSUM |
				LM_TASK_OFFLOAD_TX_UDP_CHECKSUM;
			pDevice->NoTxPseudoHdrChksum = TRUE;
		}
	}
	/* delay for 4 seconds */
	pUmDevice->delayed_link_ind = (4 * HZ) / pUmDevice->timer_interval;

	pUmDevice->adaptive_expiry = HZ / pUmDevice->timer_interval;

#if INCLUDE_TBI_SUPPORT
	if(pDevice->PollTbiLink)
		pUmDevice->poll_tbi_expiry = HZ / pUmDevice->timer_interval;
#endif

	/* Sometimes we get spurious ints. after reset when link is down. */
	/* This field tells the isr to service the int. even if there is */
	/* no status block update. */
	if (pDevice->LedMode != LED_MODE_LINK10) {
		pUmDevice->adapter_just_inited = (3 * HZ) /
			pUmDevice->timer_interval;
	}
	else {
		pUmDevice->adapter_just_inited = 0;
	}

	if (request_irq(dev->irq, &bcm5700_interrupt, SA_SHIRQ, dev->name, dev)) {
		return -EAGAIN;
	}

	pUmDevice->opened = 1;
#if TIGON3_DEBUG
	pUmDevice->tx_zc_count = 0;
	pUmDevice->tx_chksum_count = 0;
	pUmDevice->tx_himem_count = 0;
	pUmDevice->rx_good_chksum_count = 0;
	pUmDevice->rx_bad_chksum_count = 0;
#endif
	if (LM_InitializeAdapter(pDevice) != LM_STATUS_SUCCESS) {
		free_irq(dev->irq, dev);
		bcm5700_freemem(dev);
		return -EAGAIN;
	}

	if (pDevice->UndiFix) {
		printk(KERN_INFO "%s: Memory space disabled\n", dev->name);
	}

	if (memcmp(dev->dev_addr, pDevice->NodeAddress, 6)) {
		LM_SetMacAddress(pDevice, dev->dev_addr);
	}

	if (tigon3_debug > 1)
		printk(KERN_DEBUG "%s: tigon3_open() irq %d.\n", dev->name, dev->irq);

	QQ_InitQueue(&pUmDevice->rx_out_of_buf_q.Container,
        MAX_RX_PACKET_DESC_COUNT);
	netif_start_queue(dev);

#if (LINUX_VERSION_CODE < 0x020300)
	MOD_INC_USE_COUNT;
#endif

	init_timer(&pUmDevice->timer);
	pUmDevice->timer.expires = RUN_AT(pUmDevice->timer_interval);
	pUmDevice->timer.data = (unsigned long)dev;
	pUmDevice->timer.function = &bcm5700_timer;
	add_timer(&pUmDevice->timer);

	LM_EnableInterrupt(pDevice);

	return 0;
}

STATIC void
bcm5700_timer(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	long flags;
	LM_UINT32 value32;

	if (!pUmDevice->opened)
		return;

#if INCLUDE_TBI_SUPPORT
	if(pDevice->PollTbiLink && (--pUmDevice->poll_tbi_expiry == 0)) {
		value32 = REG_RD(pDevice, MacCtrl.Status);
		if (((pDevice->LinkStatus == LM_STATUS_LINK_ACTIVE) &&
			((value32 & MAC_STATUS_LINK_STATE_CHANGED) ||
			!(value32 & MAC_STATUS_PCS_SYNCED)))
			||
			((pDevice->LinkStatus != LM_STATUS_LINK_ACTIVE) &&
			(value32 & MAC_STATUS_PCS_SYNCED)))
		{
			LM_SetupPhy(pDevice);
		}
		pUmDevice->poll_tbi_expiry = HZ / pUmDevice->timer_interval;
		
        }
#endif

	if (pUmDevice->delayed_link_ind > 0) {
		if (pUmDevice->delayed_link_ind == 1)
			MM_IndicateStatus(pDevice, pDevice->LinkStatus);
		else
			pUmDevice->delayed_link_ind--;
	}
	if (pUmDevice->adapter_just_inited > 0) {
		pUmDevice->adapter_just_inited--;
		if (pDevice->EnableTbi && !pUmDevice->adapter_just_inited &&
			!pUmDevice->interrupt) {
			LM_EnableInterrupt(pDevice);
		}
	}

	if (pUmDevice->crc_counter_expiry > 0)
		pUmDevice->crc_counter_expiry--;

	if (!pUmDevice->interrupt) {
		if (!pDevice->UseTaggedStatus) {
			flags = bcm5700_lock(pUmDevice);
			if (pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) {
				/* This will generate an interrupt */
				REG_WR(pDevice, Grc.LocalCtrl,
					pDevice->GrcLocalCtrl |
					GRC_MISC_LOCAL_CTRL_SET_INT);
			}
			else {
				REG_WR(pDevice, HostCoalesce.Mode,
					HOST_COALESCE_ENABLE |
					HOST_COALESCE_NOW);
			}
			if (!(REG_RD(pDevice, DmaWrite.Mode) &
				DMA_WRITE_MODE_ENABLE)) {
				bcm5700_tx_timeout(dev);
			}
			bcm5700_unlock(pUmDevice, flags);
			if (pUmDevice->tx_queued) {
				pUmDevice->tx_queued = 0;
				netif_wake_queue(dev);
			}
		}
#if (LINUX_VERSION_CODE < 0x02032b)
		if ((QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container) !=
			pDevice->TxPacketDescCnt) &&
			((jiffies - dev->trans_start) > TX_TIMEOUT)) {

			printk(KERN_WARNING "%s: Tx hung\n", dev->name);
			bcm5700_tx_timeout(dev);
		}
#endif
	}
	if (pUmDevice->adaptive_coalesce) {
		pUmDevice->adaptive_expiry--;
		if (pUmDevice->adaptive_expiry == 0) {	
			pUmDevice->adaptive_expiry = HZ /
				pUmDevice->timer_interval;
			bcm5700_adapt_coalesce(pUmDevice);
		}
	}
	if (QQ_GetEntryCnt(&pUmDevice->rx_out_of_buf_q.Container) >=
		pUmDevice->rx_buf_repl_panic_thresh) {
		/* Generate interrupt and let isr allocate buffers */
		REG_WR(pDevice, Grc.LocalCtrl, pDevice->GrcLocalCtrl |
			GRC_MISC_LOCAL_CTRL_SET_INT);
	}

	pUmDevice->timer.expires = RUN_AT(pUmDevice->timer_interval);
	add_timer(&pUmDevice->timer);
	pUmDevice->spurious_int = 0;
}

STATIC int
bcm5700_adapt_coalesce(PUM_DEVICE_BLOCK pUmDevice)
{
	PLM_DEVICE_BLOCK pDevice = &pUmDevice->lm_dev;
	uint rx_curr_cnt, tx_curr_cnt, rx_delta, tx_delta, total_delta;
	int adapt = 0;
	long flags;

	rx_curr_cnt = pDevice->pStatsBlkVirt->ifHCInUcastPkts.Low;
	tx_curr_cnt = pDevice->pStatsBlkVirt->COSIfHCOutPkts[0].Low;
	if ((rx_curr_cnt <= pUmDevice->rx_last_cnt) ||
		(tx_curr_cnt <= pUmDevice->tx_last_cnt)) {

		/* skip if there is counter rollover */
		pUmDevice->rx_last_cnt = rx_curr_cnt;
		pUmDevice->tx_last_cnt = tx_curr_cnt;
		return 0;
	}

	rx_delta = rx_curr_cnt - pUmDevice->rx_last_cnt;
	tx_delta = tx_curr_cnt - pUmDevice->tx_last_cnt;
	total_delta = rx_delta + tx_delta;

	pUmDevice->rx_last_cnt = rx_curr_cnt;
	pUmDevice->tx_last_cnt = tx_curr_cnt;

	if (total_delta < rx_delta)
		return 0;

	if (total_delta < ADAPTIVE_LO_PKT_THRESH) {
		if (pUmDevice->rx_curr_coalesce_frames !=
			ADAPTIVE_LO_RX_MAX_COALESCED_FRAMES) {

			if (!bcm5700_trylock(pUmDevice, &flags))
				return 0;

			adapt = 1;

			pUmDevice->rx_curr_coalesce_frames =
				ADAPTIVE_LO_RX_MAX_COALESCED_FRAMES;
			pUmDevice->rx_curr_coalesce_ticks =
				ADAPTIVE_LO_RX_COALESCING_TICKS;
			pUmDevice->tx_curr_coalesce_frames =
				ADAPTIVE_LO_TX_MAX_COALESCED_FRAMES;

		}
	}
	else if (total_delta < ADAPTIVE_HI_PKT_THRESH) {
		if (pUmDevice->rx_curr_coalesce_frames !=
			DEFAULT_RX_MAX_COALESCED_FRAMES) {

			if (!bcm5700_trylock(pUmDevice, &flags))
				return 0;

			adapt = 1;

			pUmDevice->rx_curr_coalesce_frames = 
				DEFAULT_RX_MAX_COALESCED_FRAMES;
			pUmDevice->rx_curr_coalesce_ticks =
				DEFAULT_RX_COALESCING_TICKS;
			pUmDevice->tx_curr_coalesce_frames = 
				DEFAULT_TX_MAX_COALESCED_FRAMES;

		}
	}
	else {
		if (pUmDevice->rx_curr_coalesce_frames !=
			ADAPTIVE_HI_RX_MAX_COALESCED_FRAMES) {

			if (!bcm5700_trylock(pUmDevice, &flags))
				return 0;

			adapt = 1;

			pUmDevice->rx_curr_coalesce_frames = 
				ADAPTIVE_HI_RX_MAX_COALESCED_FRAMES;
			pUmDevice->rx_curr_coalesce_ticks =
				ADAPTIVE_HI_RX_COALESCING_TICKS;
			pUmDevice->tx_curr_coalesce_frames = 
				ADAPTIVE_HI_TX_MAX_COALESCED_FRAMES;

		}
	}
	if (adapt) {
    		REG_WR(pDevice, HostCoalesce.RxMaxCoalescedFrames,
			pUmDevice->rx_curr_coalesce_frames); 

		REG_WR(pDevice, HostCoalesce.RxCoalescingTicks,
			pUmDevice->rx_curr_coalesce_ticks);

		REG_WR(pDevice, HostCoalesce.TxMaxCoalescedFrames,
			pUmDevice->tx_curr_coalesce_frames); 
		bcm5700_unlock(pUmDevice, flags);
	}
	return 0;
}

STATIC void
bcm5700_tx_timeout(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;

	netif_stop_queue(dev);
	LM_DisableInterrupt(pDevice);
	LM_ResetAdapter(pDevice);	
	if (memcmp(dev->dev_addr, pDevice->NodeAddress, 6)) {
		LM_SetMacAddress(pDevice, dev->dev_addr);
	}
	LM_EnableInterrupt(pDevice);
	netif_wake_queue(dev);
}


STATIC int
bcm5700_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	PLM_PACKET pPacket;
	PUM_PACKET pUmPacket;
	long flags;
	int frag_no;
#ifdef NICE_SUPPORT
	vlan_tag_t *vlan_tag;
#endif

	if ((pDevice->LinkStatus == LM_STATUS_LINK_DOWN) || !pDevice->InitDone)
	{
		dev_kfree_skb(skb);
		return 0;
	}
	
#if (LINUX_VERSION_CODE < 0x02032b)
	if (test_and_set_bit(0, &dev->tbusy)) {
		return 1;
	}
#endif

	if (pUmDevice->do_global_lock && pUmDevice->interrupt) {
		netif_stop_queue(dev);
		pUmDevice->tx_queued = 1;
		if (!pUmDevice->interrupt) {
			netif_wake_queue(dev);
			pUmDevice->tx_queued = 0;
		}
		return 1;
	}

	pPacket = (PLM_PACKET)
		QQ_PopHead(&pDevice->TxPacketFreeQ.Container);
	if (pPacket == 0) {
		netif_stop_queue(dev);
		pUmDevice->tx_full = 1;
		if (QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container)) {
			netif_wake_queue(dev);
			pUmDevice->tx_full = 0;
		}
		return 1;
	}
	pUmPacket = (PUM_PACKET) pPacket;
	pUmPacket->skbuff = skb;

	if (skb->ip_summed == CHECKSUM_HW) {
		pPacket->Flags = SND_BD_FLAG_TCP_UDP_CKSUM;
#if TIGON3_DEBUG
		pUmDevice->tx_chksum_count++;
#endif
	}
	else {
		pPacket->Flags = 0;
	}
#if MAX_SKB_FRAGS
	frag_no = skb_shinfo(skb)->nr_frags;
#else
	frag_no = 0;
#endif
	if (atomic_read(&pDevice->SendBdLeft) < (frag_no + 1)) {
		netif_stop_queue(dev);
		pUmDevice->tx_full = 1;
		QQ_PushHead(&pDevice->TxPacketFreeQ.Container, pPacket);
		if (atomic_read(&pDevice->SendBdLeft) >= (frag_no + 1)) {
			netif_wake_queue(dev);
			pUmDevice->tx_full = 0;
		}
		return 1;
	}

	pPacket->u.Tx.FragCount = frag_no + 1;
#if TIGON3_DEBUG
	if (pPacket->u.Tx.FragCount > 1)
		pUmDevice->tx_zc_count++;
#endif

#ifdef NICE_SUPPORT
	vlan_tag = (vlan_tag_t *) &skb->cb[0];
	if (vlan_tag->signature == 0x5555) {
		pPacket->VlanTag = vlan_tag->tag;
		pPacket->Flags |= SND_BD_FLAG_VLAN_TAG;
		vlan_tag->signature = 0;
	}
#endif
	flags = bcm5700_lock(pUmDevice);
	LM_SendPacket(pDevice, pPacket);
	bcm5700_unlock(pUmDevice, flags);

#if (LINUX_VERSION_CODE < 0x02032b)
	netif_wake_queue(dev);
#endif
	dev->trans_start = jiffies;
	return 0;
}


STATIC void
bcm5700_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = (struct net_device *)dev_instance;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	LM_UINT32 oldtag, newtag;
	int repl_buf_count, i;

	if (!pDevice->InitDone)
		return;

	bcm5700_intr_lock(pUmDevice);
	if (test_and_set_bit(0, (void*)&pUmDevice->interrupt)) {
		printk(KERN_ERR "%s: Duplicate entry of the interrupt handler by "
			   "processor %d.\n",
			   dev->name, hard_smp_processor_id());
		bcm5700_intr_unlock(pUmDevice);
		return;
	}

	if (pDevice->UseTaggedStatus) {
		if ((pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) ||
			pUmDevice->adapter_just_inited) {
			MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, 1);
			oldtag = pDevice->pStatusBlkVirt->StatusTag;

			for (i = 0; ; i++) {
   				pDevice->pStatusBlkVirt->Status &=
					~STATUS_BLOCK_UPDATED;

				LM_ServiceInterrupts(pDevice);
				newtag = pDevice->pStatusBlkVirt->StatusTag;
				if ((newtag == oldtag) || (i > 50)) {
					MB_REG_WR(pDevice,
						Mailbox.Interrupt[0].Low,
						newtag << 24);
					if (pDevice->UndiFix) {
						REG_WR(pDevice, Grc.LocalCtrl,
						pDevice->GrcLocalCtrl | 0x2);
					}
					break;
				}
				oldtag = newtag;
			}
		}
	}
	else if ((pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) ||
			pUmDevice->adapter_just_inited) {
		do {
			uint dummy;

			MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, 1);
   			pDevice->pStatusBlkVirt->Status &= ~STATUS_BLOCK_UPDATED;
			LM_ServiceInterrupts(pDevice);
			MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, 0);
			dummy = MB_REG_RD(pDevice, Mailbox.Interrupt[0].Low);
		}
		while (pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED);
		if (pDevice->UndiFix) {
			REG_WR(pDevice, Grc.LocalCtrl,
				pDevice->GrcLocalCtrl | 0x2);
		}
	}
	if (pUmDevice->adapter_just_inited && pDevice->EnableTbi) {
		if (pDevice->LinkStatus != LM_STATUS_LINK_ACTIVE) {
			pUmDevice->spurious_int++;
			if (pUmDevice->spurious_int > 25) {
				LM_DisableInterrupt(pDevice);
    				REG_WR(pDevice, MacCtrl.Mode, pDevice->MacMode |
					MAC_MODE_LINK_POLARITY);
				MM_Wait(1);
				pUmDevice->spurious_int = 0;
    				REG_WR(pDevice, MacCtrl.Mode, pDevice->MacMode);
				if (pUmDevice->adapter_just_inited == 0)
					LM_EnableInterrupt(pDevice);
			}
		}
	}
#ifdef TASKLET
	repl_buf_count = QQ_GetEntryCnt(&pUmDevice->rx_out_of_buf_q.Container);
	if (repl_buf_count >= pUmDevice->rx_buf_repl_thresh) {
		if ((repl_buf_count >= pUmDevice->rx_buf_repl_panic_thresh) &&
			(!test_and_set_bit(0, &pUmDevice->tasklet_busy))) {
			replenish_rx_buffers(pUmDevice);
			clear_bit(0, (void*)&pUmDevice->tasklet_busy);
		}
		else if (!pUmDevice->tasklet_pending) {
			pUmDevice->tasklet_pending = 1;
			tasklet_schedule(&pUmDevice->tasklet);
		}
	}
#else
	if (QQ_GetEntryCnt(&pUmDevice->rx_out_of_buf_q.Container)) {
		replenish_rx_buffers(pUmDevice);
	}

	if (QQ_GetEntryCnt(&pDevice->RxPacketFreeQ.Container)) {
		LM_QueueRxPackets(pDevice);
	}
#endif

	clear_bit(0, (void*)&pUmDevice->interrupt);
	bcm5700_intr_unlock(pUmDevice);
	if (pUmDevice->tx_queued) {
		pUmDevice->tx_queued = 0;
		netif_wake_queue(dev);
	}
	return;
}


STATIC void
bcm5700_tasklet(unsigned long data)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)data;

	/* RH 7.2 Beta 3 tasklets are reentrant */
	if (test_and_set_bit(0, &pUmDevice->tasklet_busy)) {
		pUmDevice->tasklet_pending = 0;
		return;
	}

	pUmDevice->tasklet_pending = 0;
	replenish_rx_buffers(pUmDevice);
	clear_bit(0, &pUmDevice->tasklet_busy);
}

STATIC int
bcm5700_close(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;

#if (LINUX_VERSION_CODE < 0x02032b)
	dev->start = 0;
#endif
	netif_stop_queue(dev);
	pUmDevice->opened = 0;

	if (tigon3_debug > 1)
		printk(KERN_DEBUG "%s: Shutting down Tigon3\n",
			   dev->name);

	LM_DisableInterrupt(pDevice);
#ifdef TASKLET
/*	tasklet_disable(&pUmDevice->tasklet); */
	tasklet_kill(&pUmDevice->tasklet);
#endif
	LM_Halt(pDevice);
	pDevice->InitDone = 0;
	del_timer(&pUmDevice->timer);

	free_irq(dev->irq, dev);
#if (LINUX_VERSION_CODE < 0x020300)
	MOD_DEC_USE_COUNT;
#endif
	LM_SetPowerState(pDevice, LM_POWER_STATE_D3);
	bcm5700_freemem(dev);

	return 0;
}

STATIC int
bcm5700_freemem(struct net_device *dev)
{
	int i;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;

	for (i = 0; i < pUmDevice->mem_list_num; i++) {
		if (pUmDevice->mem_size_list[i] == 0) {
			kfree(pUmDevice->mem_list[i]);
		}
		else {
			pci_free_consistent(pUmDevice->pdev,
				(size_t) pUmDevice->mem_size_list[i],
				pUmDevice->mem_list[i],
				pUmDevice->dma_list[i]);
		}
	}
	pUmDevice->mem_list_num = 0;
	return 0;
}

LM_UINT32
bcm5700_crc_count(PUM_DEVICE_BLOCK pUmDevice)
{
	PLM_DEVICE_BLOCK pDevice = &pUmDevice->lm_dev;
	LM_UINT32 Value32;
	PT3_STATS_BLOCK pStats = (PT3_STATS_BLOCK) pDevice->pStatsBlkVirt;
	unsigned long flags;

#if INCLUDE_TBI_SUPPORT
	if(pDevice->EnableTbi)
		return (pStats->dot3StatsFCSErrors.Low);
#endif
	if (T3_ASIC_REV(pDevice->ChipRevId) == T3_ASIC_REV_5700 ||
		T3_ASIC_REV(pDevice->ChipRevId) == T3_ASIC_REV_5701) {

		if (!pUmDevice->opened || pUmDevice->adapter_just_inited)
			return 0;

		/* regulate MDIO access during run time */
		if (pUmDevice->crc_counter_expiry > 0)
			return pDevice->PhyCrcCount;

		pUmDevice->crc_counter_expiry = (5 * HZ) /
			pUmDevice->timer_interval;

		flags = bcm5700_lock(pUmDevice);
		LM_ReadPhy(pDevice, 0x1e, &Value32);
		if ((Value32 & 0x8000) == 0)
			LM_WritePhy(pDevice, 0x1e, Value32 | 0x8000);
		LM_ReadPhy(pDevice, 0x14, &Value32);
		bcm5700_unlock(pUmDevice, flags);
		/* Sometimes data on the MDIO bus can be corrupted */
		if (Value32 != 0xffff)
			pDevice->PhyCrcCount += Value32;
		return pDevice->PhyCrcCount;
	}
	else if (pStats == 0) {
		return 0;
	}
	else {
		return (pStats->dot3StatsFCSErrors.Low);
	}
}

STATIC struct net_device_stats *
bcm5700_get_stats(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	PT3_STATS_BLOCK pStats = (PT3_STATS_BLOCK) pDevice->pStatsBlkVirt;
	struct net_device_stats *p_netstats = &pUmDevice->stats;

	if (pStats == 0)
		return p_netstats;

	/* Get stats from LM */
	p_netstats->rx_packets = pStats->ifHCInUcastPkts.Low +
			pStats->ifHCInMulticastPkts.Low +
			pStats->ifHCInBroadcastPkts.Low;
	p_netstats->tx_packets = pStats->COSIfHCOutPkts[0].Low;
	p_netstats->rx_bytes = pStats->ifHCInOctets.Low;
	p_netstats->tx_bytes = pStats->ifHCOutOctets.Low;
	p_netstats->tx_errors = pStats->dot3StatsInternalMacTransmitErrors.Low +
			pStats->dot3StatsCarrierSenseErrors.Low +
			pStats->ifOutDiscards.Low +
			pStats->ifOutErrors.Low;
	p_netstats->multicast = pStats->ifHCInMulticastPkts.Low;
	p_netstats->collisions = pStats->etherStatsCollisions.Low;
	p_netstats->rx_length_errors = pStats->dot3StatsFramesTooLong.Low +
			pStats->etherStatsUndersizePkts.Low;
	p_netstats->rx_over_errors = pStats->nicNoMoreRxBDs.Low;
	p_netstats->rx_frame_errors = pStats->dot3StatsAlignmentErrors.Low;
	p_netstats->rx_crc_errors = bcm5700_crc_count(pUmDevice);
	p_netstats->rx_errors = p_netstats->rx_length_errors +
				p_netstats->rx_over_errors +
				p_netstats->rx_frame_errors +
				p_netstats->rx_crc_errors +
				pStats->etherStatsFragments.Low +
				pStats->etherStatsJabbers.Low;
	
	p_netstats->tx_aborted_errors = pStats->ifOutDiscards.Low;
	p_netstats->tx_carrier_errors = pStats->dot3StatsCarrierSenseErrors.Low;

	return p_netstats;
}

#ifdef SIOCETHTOOL
static int netdev_ethtool_ioctl(struct net_device *dev, void *useraddr)
{
	struct ethtool_cmd ethcmd;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
		
	if (copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)))
		return -EFAULT;

        switch (ethcmd.cmd) {
#ifdef ETHTOOL_GDRVINFO
        case ETHTOOL_GDRVINFO: {
		struct ethtool_drvinfo info = {ETHTOOL_GDRVINFO};

		strcpy(info.driver,  bcm5700_driver);
#if INCLUDE_5701_AX_FIX
		if(pDevice->ChipRevId == T3_CHIP_ID_5701_A0) {
			extern int t3FwReleaseMajor;
			extern int t3FwReleaseMinor;
			extern int t3FwReleaseFix;

			sprintf(info.fw_version, "%i.%i.%i",
				t3FwReleaseMajor, t3FwReleaseMinor, 
				t3FwReleaseFix);
		}
#endif
		strcpy(info.version, bcm5700_version);
		strcpy(info.bus_info, pUmDevice->pdev->slot_name);
		if (copy_to_user(useraddr, &info, sizeof(info)))
			return -EFAULT;
		return 0;
	}
#endif
        case ETHTOOL_GSET: {
		if (pDevice->EnableTbi) {
			ethcmd.supported =
				(SUPPORTED_1000baseT_Full |
				SUPPORTED_Autoneg);
			ethcmd.supported |= SUPPORTED_FIBRE;
			ethcmd.port = PORT_FIBRE;
		}
		else {
			ethcmd.supported =
				(SUPPORTED_10baseT_Half |
				SUPPORTED_10baseT_Full |
				SUPPORTED_100baseT_Half |
				SUPPORTED_100baseT_Full |
				SUPPORTED_1000baseT_Half |
				SUPPORTED_1000baseT_Full |
				SUPPORTED_Autoneg);
			ethcmd.supported |= SUPPORTED_TP;
			ethcmd.port = PORT_TP;
		}

		ethcmd.transceiver = XCVR_INTERNAL;
		ethcmd.phy_address = 0;

		if (pUmDevice->line_speed == 1000)
			ethcmd.speed = SPEED_1000;
		else if (pUmDevice->line_speed == 100)
			ethcmd.speed = SPEED_100;
		else if (pUmDevice->line_speed == 10)
			ethcmd.speed = SPEED_10;
		else
			ethcmd.speed = 0;

		if (pDevice->DuplexMode == LM_DUPLEX_MODE_FULL)
			ethcmd.duplex = DUPLEX_FULL;
		else
			ethcmd.duplex = DUPLEX_HALF;

		if (pDevice->DisableAutoNeg == FALSE) {
			ethcmd.autoneg = AUTONEG_ENABLE;
			ethcmd.advertising = ADVERTISED_Autoneg;
			if (pDevice->EnableTbi) {
				ethcmd.advertising |=
					ADVERTISED_1000baseT_Full |
					ADVERTISED_FIBRE;
			}
			else {
				ethcmd.advertising |=
					ADVERTISED_TP;
				if (pDevice->advertising &
					PHY_AN_AD_10BASET_HALF) {

					ethcmd.advertising |=
						ADVERTISED_10baseT_Half;
				}
				if (pDevice->advertising &
					PHY_AN_AD_10BASET_FULL) {

					ethcmd.advertising |=
						ADVERTISED_10baseT_Full;
				}
				if (pDevice->advertising &
					PHY_AN_AD_100BASETX_HALF) {

					ethcmd.advertising |=
						ADVERTISED_100baseT_Half;
				}
				if (pDevice->advertising &
					PHY_AN_AD_100BASETX_FULL) {

					ethcmd.advertising |=
						ADVERTISED_100baseT_Full;
				}
				if (pDevice->advertising1000 &
					BCM540X_AN_AD_1000BASET_HALF) {

					ethcmd.advertising |=
						ADVERTISED_1000baseT_Half;
				}
				if (pDevice->advertising1000 &
					BCM540X_AN_AD_1000BASET_FULL) {

					ethcmd.advertising |=
						ADVERTISED_1000baseT_Full;
				}
			}
		}
		else {
			ethcmd.autoneg = AUTONEG_DISABLE;
			ethcmd.advertising = 0;
		}

		ethcmd.maxtxpkt = pDevice->TxMaxCoalescedFrames;
		ethcmd.maxrxpkt = pDevice->RxMaxCoalescedFrames;

		if(copy_to_user(useraddr, &ethcmd, sizeof(ethcmd)))
			return -EFAULT;
		return 0;
	}
	case ETHTOOL_SSET: {
		if(!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (ethcmd.autoneg == AUTONEG_ENABLE) {
			if (pDevice->EnableTbi) {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_AUTO;
			}
			else {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_AUTO;
			}
			pDevice->DisableAutoNeg = FALSE;
		}
		else if (ethcmd.speed == SPEED_1000) {
			if (!pDevice->EnableTbi)
				return -EINVAL;
			pDevice->RequestedMediaType =
				LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS_FULL_DUPLEX;
			pDevice->DisableAutoNeg = TRUE;
		}
		else if (ethcmd.speed == SPEED_100) {
			if (ethcmd.duplex == DUPLEX_FULL) {
				pDevice->RequestedMediaType =
				LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS_FULL_DUPLEX;
			}
			else {
				pDevice->RequestedMediaType =
				LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS;
			}
			pDevice->DisableAutoNeg = TRUE;
		}
		else if (ethcmd.speed == SPEED_10) {
			if (ethcmd.duplex == DUPLEX_FULL) {
				pDevice->RequestedMediaType =
				LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS_FULL_DUPLEX;
			}
			else {
				pDevice->RequestedMediaType =
				LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS;
			}
			pDevice->DisableAutoNeg = TRUE;
		}
		else {
			return -EINVAL;
		}
		LM_SetupPhy(pDevice);
		return 0;
	}
#ifdef ETHTOOL_GWOL
	case ETHTOOL_GWOL: {
		struct ethtool_wolinfo wol = {ETHTOOL_GWOL};

		if (pDevice->EnableTbi) {
			wol.supported = 0;
			wol.wolopts = 0;
		}
		else {
			wol.supported = WAKE_MAGIC;
			if (pDevice->WakeUpMode == LM_WAKE_UP_MODE_MAGIC_PACKET)
			{
				wol.wolopts = WAKE_MAGIC;
			}
			else {
				wol.wolopts = 0;
			}
		}
		if (copy_to_user(useraddr, &wol, sizeof(wol)))
			return -EFAULT;
		return 0;
	}
	case ETHTOOL_SWOL: {
		struct ethtool_wolinfo wol;

		if(!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (copy_from_user(&wol, useraddr, sizeof(wol)))
			return -EFAULT;
		if (pDevice->EnableTbi && wol.wolopts)
			return -EINVAL;
		
		if ((wol.wolopts & ~WAKE_MAGIC) != 0) {
			return -EINVAL;
		}
		if (wol.wolopts & WAKE_MAGIC) {
			pDevice->WakeUpModeCap = LM_WAKE_UP_MODE_MAGIC_PACKET;
			pDevice->WakeUpMode = LM_WAKE_UP_MODE_MAGIC_PACKET;
		}
		else {
			pDevice->WakeUpModeCap = LM_WAKE_UP_MODE_NONE;
			pDevice->WakeUpMode = LM_WAKE_UP_MODE_NONE;
		}
		return 0;
        }
#endif
#ifdef ETHTOOL_GLINK
	case ETHTOOL_GLINK: {
		struct ethtool_value edata = {ETHTOOL_GLINK};

		if (pDevice->LinkStatus == LM_STATUS_LINK_ACTIVE)
			edata.data =  1;
		else
			edata.data =  0;
		if (copy_to_user(useraddr, &edata, sizeof(edata)))
			return -EFAULT;
		return 0;
	}
#endif
#ifdef ETHTOOL_NWAY_RST
	case ETHTOOL_NWAY_RST: {
		LM_UINT32 phyctrl;

		if (pDevice->DisableAutoNeg) {
			return -EINVAL;
		}
		if (pDevice->EnableTbi) {
			LM_SetupPhy(pDevice);
		}
		else {
			LM_ReadPhy(pDevice, PHY_CTRL_REG, &phyctrl);
			LM_WritePhy(pDevice, PHY_CTRL_REG, phyctrl |
				PHY_CTRL_AUTO_NEG_ENABLE |
				PHY_CTRL_RESTART_AUTO_NEG);
		}
		return 0;
	}
#endif
	}
	
	return -EOPNOTSUPP;
}
#endif

/* Provide ioctl() calls to examine the MII xcvr state. */
STATIC int bcm5700_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	u16 *data = (u16 *)&rq->ifr_data;
	u32 value;
	unsigned long flags;

	switch(cmd) {
	case SIOCDEVPRIVATE:		/* Get the address of the PHY in use. */
		data[0] = pDevice->PhyAddr;
	case SIOCDEVPRIVATE+1:		/* Read the specified MII register. */
		flags = bcm5700_lock(pUmDevice);
		LM_ReadPhy(pDevice, data[1] & 0x1f, (LM_UINT32 *) &value);
		bcm5700_unlock(pUmDevice, flags);
		data[3] = value & 0xffff;
		return 0;
	case SIOCDEVPRIVATE+2:		/* Write the specified MII register */
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		flags = bcm5700_lock(pUmDevice);
		LM_WritePhy(pDevice, data[1] & 0x1f, data[2]);
		bcm5700_unlock(pUmDevice, flags);
		return 0;
#ifdef NICE_SUPPORT
	case SIOCNICE:
		{
			struct nice_req* nrq;

			if (!capable(CAP_NET_ADMIN))
				return -EPERM;

			nrq = (struct nice_req*)&rq->ifr_ifru;
			if( nrq->cmd == NICE_CMD_QUERY_SUPPORT ) {
				nrq->nrq_magic = NICE_DEVICE_MAGIC;
				nrq->nrq_support_rx = 1;
				nrq->nrq_support_vlan = 1;
				nrq->nrq_support_get_speed = 1;
				return 0;
			}
			else if( nrq->cmd == NICE_CMD_SET_RX ) {
				pUmDevice->nice_rx = nrq->nrq_rx;
				pUmDevice->nice_ctx = nrq->nrq_ctx;
				return 0;
			}
			else if( nrq->cmd == NICE_CMD_GET_RX ) {
				nrq->nrq_rx = pUmDevice->nice_rx;
				nrq->nrq_ctx = pUmDevice->nice_ctx;
				return 0;
			}
			else if( nrq->cmd == NICE_CMD_GET_SPEED ) {
				nrq->nrq_speed = pUmDevice->line_speed;
				return 0;
			}
			else if( nrq->cmd == NICE_CMD_BLINK_LED ) {
				return LM_BlinkLED(pDevice, nrq->nrq_blink_time);
			}
			break;
		}
#endif /* NICE_SUPPORT */
#ifdef SIOCETHTOOL
	case SIOCETHTOOL:
		return netdev_ethtool_ioctl(dev, (void *) rq->ifr_data);
#endif
	default:
		return -EOPNOTSUPP;
	}
	return -EOPNOTSUPP;
}

STATIC void bcm5700_set_rx_mode(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	int i;
	struct dev_mc_list *mclist;

	LM_MulticastClear(pDevice);
	for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
			 i++, mclist = mclist->next) {
		LM_MulticastAdd(pDevice, (PLM_UINT8) &mclist->dmi_addr);
	}
	if (dev->flags & IFF_ALLMULTI) {
		if (!(pDevice->ReceiveMask & LM_ACCEPT_ALL_MULTICAST)) {
			LM_SetReceiveMask(pDevice,
				pDevice->ReceiveMask | LM_ACCEPT_ALL_MULTICAST);
		}
	}
	else if (pDevice->ReceiveMask & LM_ACCEPT_ALL_MULTICAST) {
		LM_SetReceiveMask(pDevice,
			pDevice->ReceiveMask & ~LM_ACCEPT_ALL_MULTICAST);
	}
	if (dev->flags & IFF_PROMISC) {
		if (!(pDevice->RxMode & RX_MODE_PROMISCUOUS_MODE)) {
			LM_SetReceiveMask(pDevice,
				pDevice->ReceiveMask | LM_PROMISCUOUS_MODE);
		}
	}
	else if (pDevice->RxMode & RX_MODE_PROMISCUOUS_MODE) {
		LM_SetReceiveMask(pDevice,
			pDevice->ReceiveMask & ~LM_PROMISCUOUS_MODE);
	}
}

/*
 * Set the hardware MAC address.
 */
STATIC int bcm5700_set_mac_addr(struct net_device *dev, void *p)
{
	struct sockaddr *addr=p;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) dev->priv;

	if (netif_running(dev))
		return -EBUSY;
	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);
	LM_SetMacAddress(pDevice, dev->dev_addr);
	return 0;
}


#if (LINUX_VERSION_CODE < 0x020300)
#ifdef MODULE
int init_module(void)
{
	return bcm5700_probe(NULL);
}

void cleanup_module(void)
{
	struct net_device *next_dev;
	PUM_DEVICE_BLOCK pUmDevice;

	/* No need to check MOD_IN_USE, as sys_delete_module() checks. */
	while (root_tigon3_dev) {
		pUmDevice = (PUM_DEVICE_BLOCK)root_tigon3_dev->priv;
#ifdef CONFIG_PROC_FS
		bcm5700_proc_remove_dev(root_tigon3_dev); 
#endif
		next_dev = pUmDevice->next_module;
		unregister_netdev(root_tigon3_dev);
		if (pUmDevice->lm_dev.pMappedMemBase)
			iounmap(pUmDevice->lm_dev.pMappedMemBase);
		kfree(root_tigon3_dev);
		root_tigon3_dev = next_dev;
	}
}

#endif  /* MODULE */
#else	/* LINUX_VERSION_CODE < 0x020300 */

#if (LINUX_VERSION_CODE >= 0x020406)
static int bcm5700_suspend (struct pci_dev *pdev, u32 state)
#else
static void bcm5700_suspend (struct pci_dev *pdev)
#endif
{
	struct net_device *dev = (struct net_device *) pci_get_drvdata(pdev);
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) dev->priv;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;

	if (!netif_running(dev))
#if (LINUX_VERSION_CODE >= 0x020406)
		return 0;
#else
		return;
#endif

	LM_DisableInterrupt(pDevice);
	netif_device_detach (dev);

	/* Disable interrupts, stop Tx and Rx. */
	LM_Halt(pDevice);
	LM_SetPowerState(pDevice, LM_POWER_STATE_D3);

/*	pci_power_off(pdev, -1);*/
#if (LINUX_VERSION_CODE >= 0x020406)
		return 0;
#endif
}


#if (LINUX_VERSION_CODE >= 0x020406)
static int bcm5700_resume(struct pci_dev *pdev)
#else
static void bcm5700_resume(struct pci_dev *pdev)
#endif
{
	struct net_device *dev = (struct net_device *) pci_get_drvdata(pdev);
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) dev->priv;

	if (!netif_running(dev))
#if (LINUX_VERSION_CODE >= 0x020406)
		return 0;
#else
		return;
#endif
/*	pci_power_on(pdev);*/
	netif_device_attach(dev);
	LM_InitializeAdapter(pDevice);
	if (memcmp(dev->dev_addr, pDevice->NodeAddress, 6)) {
		LM_SetMacAddress(pDevice, dev->dev_addr);
	}
	LM_EnableInterrupt(pDevice);
#if (LINUX_VERSION_CODE >= 0x020406)
	return 0;
#endif
}


static struct pci_driver bcm5700_pci_driver = {
	name:		bcm5700_driver,
	id_table:	bcm5700_pci_tbl,
	probe:		bcm5700_init_one,
	remove:		bcm5700_remove_one,
	suspend:	bcm5700_suspend,
	resume:		bcm5700_resume,
};


static int __init bcm5700_init_module (void)
{
	return pci_module_init(&bcm5700_pci_driver);
}


static void __exit bcm5700_cleanup_module (void)
{
	pci_unregister_driver(&bcm5700_pci_driver);
}


module_init(bcm5700_init_module);
module_exit(bcm5700_cleanup_module);
#endif

/*
 * Middle Module
 *
 */


LM_STATUS
MM_ReadConfig16(PLM_DEVICE_BLOCK pDevice, LM_UINT32 Offset,
	LM_UINT16 *pValue16)
{
	UM_DEVICE_BLOCK *pUmDevice;

	pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
	pci_read_config_word(pUmDevice->pdev, Offset, (u16 *) pValue16);
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_ReadConfig32(PLM_DEVICE_BLOCK pDevice, LM_UINT32 Offset,
	LM_UINT32 *pValue32)
{
	UM_DEVICE_BLOCK *pUmDevice;

	pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
	pci_read_config_dword(pUmDevice->pdev, Offset, (u32 *) pValue32);
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_WriteConfig16(PLM_DEVICE_BLOCK pDevice, LM_UINT32 Offset,
	LM_UINT16 Value16)
{
	UM_DEVICE_BLOCK *pUmDevice;

	pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
	pci_write_config_word(pUmDevice->pdev, Offset, Value16);
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_WriteConfig32(PLM_DEVICE_BLOCK pDevice, LM_UINT32 Offset,
	LM_UINT32 Value32)
{
	UM_DEVICE_BLOCK *pUmDevice;

	pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
	pci_write_config_dword(pUmDevice->pdev, Offset, Value32);
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_AllocateSharedMemory(PLM_DEVICE_BLOCK pDevice, LM_UINT32 BlockSize,
	PLM_VOID *pMemoryBlockVirt, PLM_PHYSICAL_ADDRESS pMemoryBlockPhy,
	LM_BOOL Cached)
{
	PLM_VOID pvirt;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
	dma_addr_t mapping;

	pvirt = pci_alloc_consistent(pUmDevice->pdev, BlockSize,
					       &mapping);
	if (!pvirt) {
		return LM_STATUS_FAILURE;
	}
	pUmDevice->mem_list[pUmDevice->mem_list_num] = pvirt;
	pUmDevice->dma_list[pUmDevice->mem_list_num] = mapping;
	pUmDevice->mem_size_list[pUmDevice->mem_list_num++] = BlockSize;
	memset(pvirt, 0, BlockSize);
	*pMemoryBlockVirt = (PLM_VOID) pvirt;
	MM_SetAddr(pMemoryBlockPhy, mapping);
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_AllocateMemory(PLM_DEVICE_BLOCK pDevice, LM_UINT32 BlockSize,
	PLM_VOID *pMemoryBlockVirt)
{
	PLM_VOID pvirt;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;


	/* Maximum in slab.c */
	if (BlockSize > 131072) {
		goto MM_Alloc_error;
	}

	pvirt = kmalloc(BlockSize, GFP_KERNEL);
	if (!pvirt) {
		goto MM_Alloc_error;
	}
	pUmDevice->mem_list[pUmDevice->mem_list_num] = pvirt;
	pUmDevice->dma_list[pUmDevice->mem_list_num] = 0;
	pUmDevice->mem_size_list[pUmDevice->mem_list_num++] = 0;
	/* mem_size_list[i] == 0 indicates that the memory should be freed */
	/* using kfree */
	memset(pvirt, 0, BlockSize);
	*pMemoryBlockVirt = pvirt;
	return LM_STATUS_SUCCESS;

MM_Alloc_error:
	printk(KERN_WARNING "%s: Memory allocation failed - buffer parameters may be set too high\n", pUmDevice->dev->name);
	return LM_STATUS_FAILURE;
}

LM_STATUS
MM_MapMemBase(PLM_DEVICE_BLOCK pDevice)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;

	pDevice->pMappedMemBase = ioremap_nocache(
		pci_resource_start(pUmDevice->pdev, 0), sizeof(T3_STD_MEM_MAP));
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_InitializeUmPackets(PLM_DEVICE_BLOCK pDevice)
{
	int i;
	struct sk_buff *skb;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
	PUM_PACKET pUmPacket;
	PLM_PACKET pPacket;

	for (i = 0; i < pDevice->RxPacketDescCnt; i++) {
		pPacket = QQ_PopHead(&pDevice->RxPacketFreeQ.Container);
		pUmPacket = (PUM_PACKET) pPacket;
		if (pPacket == 0) {
			printk(KERN_DEBUG "Bad RxPacketFreeQ\n");
		}
		skb = dev_alloc_skb(pPacket->u.Rx.RxBufferSize + 2);
		if (skb == 0) {
			pUmPacket->skbuff = 0;
			QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
			continue;
		}
		pUmPacket->skbuff = skb;
		skb->dev = pUmDevice->dev;
		skb_reserve(skb, pUmDevice->rx_buf_align);
		QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	}
	if (T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5700) {
		/* reallocate buffers in the ISR */
		pUmDevice->rx_buf_repl_thresh = 0;
		pUmDevice->rx_buf_repl_panic_thresh = 0;
	}
	else if (T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5703) {
		pUmDevice->rx_buf_repl_thresh = pDevice->RxPacketDescCnt / 10;
		pUmDevice->rx_buf_repl_panic_thresh =
			pDevice->RxPacketDescCnt / 3;
	}
	else {
		pUmDevice->rx_buf_repl_thresh = pDevice->RxPacketDescCnt / 8;
		pUmDevice->rx_buf_repl_panic_thresh =
			pDevice->RxPacketDescCnt / 2;
	}
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_GetConfig(PLM_DEVICE_BLOCK pDevice)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
	int index = pUmDevice->index;

	if (auto_speed[index] == 0)
		pDevice->DisableAutoNeg = TRUE;
	else
		pDevice->DisableAutoNeg = FALSE;

	if (line_speed[index] == 0) {
		pDevice->RequestedMediaType =
				LM_REQUESTED_MEDIA_TYPE_AUTO;
		pDevice->DisableAutoNeg = FALSE;
	}
	else {
		if (line_speed[index] == 1000) {
			if (pDevice->EnableTbi) {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_FIBER_1000MBPS_FULL_DUPLEX;
			}
			else if (full_duplex[index]) {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS_FULL_DUPLEX;
			}
			else {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS;
			}
			if (!pDevice->EnableTbi)
				pDevice->DisableAutoNeg = FALSE;
		}
		else if (line_speed[index] == 100) {
			if (full_duplex[index]) {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS_FULL_DUPLEX;
			}
			else {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS;
			}
		}
		else if (line_speed[index] == 10) {
			if (full_duplex[index]) {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS_FULL_DUPLEX;
			}
			else {
				pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS;
			}
		}
		else {
			pDevice->RequestedMediaType =
					LM_REQUESTED_MEDIA_TYPE_AUTO;
			pDevice->DisableAutoNeg = FALSE;
			printk(KERN_WARNING "%s: Invalid line_speed parameter (%d), using 0\n", pUmDevice->dev->name, line_speed[index]);
		}

	}
	pDevice->FlowControlCap = 0;
	if (rx_flow_control[index] != 0) {
		pDevice->FlowControlCap |= LM_FLOW_CONTROL_RECEIVE_PAUSE;
	}
	if (tx_flow_control[index] != 0) {
		pDevice->FlowControlCap |= LM_FLOW_CONTROL_TRANSMIT_PAUSE;
	}
	if (auto_flow_control[index] != 0) {
		if (pDevice->DisableAutoNeg == FALSE) {

			pDevice->FlowControlCap |= LM_FLOW_CONTROL_AUTO_PAUSE;
			if ((tx_flow_control[index] == 0) &&
				(rx_flow_control[index] == 0)) {

				pDevice->FlowControlCap |=
					LM_FLOW_CONTROL_TRANSMIT_PAUSE |
					LM_FLOW_CONTROL_RECEIVE_PAUSE;
			}
		}
		else {
			printk(KERN_WARNING "%s: Conflicting auto_flow_control parameter (%d), using 0\n",
				pUmDevice->dev->name, auto_flow_control[index]);
		}

	}

	if (pUmDevice->dev->mtu > 1500) {
		pDevice->RxMtu = pUmDevice->dev->mtu + 14;
	}

	if (T3_ASIC_REV(pDevice->ChipRevId) != T3_ASIC_REV_5700) {
		pDevice->UseTaggedStatus = TRUE;
		pUmDevice->timer_interval = HZ;
	}
	else {
		pUmDevice->timer_interval = HZ/10;
	}

	if ((tx_pkt_desc_cnt[index] == 0) ||
		(tx_pkt_desc_cnt[index] > MAX_TX_PACKET_DESC_COUNT)) {

		printk(KERN_WARNING "%s: Invalid tx_pkt_desc_cnt parameter (%d), using %d\n",
			pUmDevice->dev->name, tx_pkt_desc_cnt[index],
			DEFAULT_TX_PACKET_DESC_COUNT);

		tx_pkt_desc_cnt[index] = DEFAULT_TX_PACKET_DESC_COUNT;
	}
	pDevice->TxPacketDescCnt = tx_pkt_desc_cnt[index];
	if ((rx_std_desc_cnt[index] == 0) ||
		(rx_std_desc_cnt[index] >= T3_STD_RCV_RCB_ENTRY_COUNT)) {

		printk(KERN_WARNING "%s: Invalid rx_std_desc_cnt parameter (%d), using %d\n",
			pUmDevice->dev->name, rx_std_desc_cnt[index],
			DEFAULT_RX_PACKET_DESC_COUNT);

		rx_std_desc_cnt[index] = DEFAULT_RX_PACKET_DESC_COUNT;
	}
	pDevice->RxStdDescCnt = rx_std_desc_cnt[index];

	if (mtu[index] <= 1514) {
		rx_jumbo_desc_cnt[index] = 0;
	}
	else if ((rx_jumbo_desc_cnt[index] == 0) ||
		(rx_jumbo_desc_cnt[index] >= T3_JUMBO_RCV_RCB_ENTRY_COUNT)) {

		printk(KERN_WARNING "%s: Invalid rx_jumbo_desc_cnt parameter (%d), using %d\n",
			pUmDevice->dev->name, rx_jumbo_desc_cnt[index],
			DEFAULT_JUMBO_RCV_DESC_COUNT);

		rx_jumbo_desc_cnt[index] = DEFAULT_JUMBO_RCV_DESC_COUNT;
	}
	pDevice->RxJumboDescCnt = rx_jumbo_desc_cnt[index];

	pUmDevice->adaptive_coalesce = adaptive_coalesce[index];
	if (!pUmDevice->adaptive_coalesce) {
		if (rx_coalesce_ticks[index] > MAX_RX_COALESCING_TICKS) {

			printk(KERN_WARNING "%s: Invalid rx_coalesce_ticks parameter (%d), using %d\n",
				pUmDevice->dev->name,
				rx_coalesce_ticks[index],
				MAX_RX_COALESCING_TICKS);

			rx_coalesce_ticks[index] = MAX_RX_COALESCING_TICKS;
		}
		else if ((rx_coalesce_ticks[index] == 0) &&
			(rx_max_coalesce_frames[index] == 0)) {

			printk(KERN_WARNING "%s: Conflicting rx_coalesce_ticks (0) and rx_max_coalesce_frames (0) parameters, using %d and %d respectively\n",
				pUmDevice->dev->name,
				DEFAULT_RX_COALESCING_TICKS,
				DEFAULT_RX_MAX_COALESCED_FRAMES);

			rx_coalesce_ticks[index] = DEFAULT_RX_COALESCING_TICKS;
			rx_max_coalesce_frames[index] =
				DEFAULT_RX_MAX_COALESCED_FRAMES;
		}
		pDevice->RxCoalescingTicks = rx_coalesce_ticks[index];
		pUmDevice->rx_curr_coalesce_ticks = pDevice->RxCoalescingTicks;

		if (rx_max_coalesce_frames[index] > MAX_RX_MAX_COALESCED_FRAMES)
		{
			printk(KERN_WARNING "%s: Invalid rx_max_coalesce_frames parameter (%d), using %d\n",
				pUmDevice->dev->name,
				rx_max_coalesce_frames[index],
				MAX_RX_MAX_COALESCED_FRAMES);

			rx_max_coalesce_frames[index] =
				MAX_RX_MAX_COALESCED_FRAMES;
		}
		pDevice->RxMaxCoalescedFrames = rx_max_coalesce_frames[index];
		pUmDevice->rx_curr_coalesce_frames =
			pDevice->RxMaxCoalescedFrames;

		if (tx_coalesce_ticks[index] > MAX_TX_COALESCING_TICKS) {
			printk(KERN_WARNING "%s: Invalid tx_coalesce_ticks parameter (%d), using %d\n",
				pUmDevice->dev->name,
				tx_coalesce_ticks[index],
				MAX_TX_COALESCING_TICKS);

			tx_coalesce_ticks[index] = MAX_TX_COALESCING_TICKS;
		}
		else if ((tx_coalesce_ticks[index] == 0) &&
			(tx_max_coalesce_frames[index] == 0)) {

			printk(KERN_WARNING "%s: Conflicting tx_coalesce_ticks (0) and tx_max_coalesce_frames (0) parameters, using %d and %d respectively\n",
				pUmDevice->dev->name,
				DEFAULT_TX_COALESCING_TICKS,
				DEFAULT_TX_MAX_COALESCED_FRAMES);

			tx_coalesce_ticks[index] = DEFAULT_TX_COALESCING_TICKS;
			tx_max_coalesce_frames[index] =
				DEFAULT_TX_MAX_COALESCED_FRAMES;
		}
		pDevice->TxCoalescingTicks = tx_coalesce_ticks[index];
		if (tx_max_coalesce_frames[index] > MAX_TX_MAX_COALESCED_FRAMES) {
			printk(KERN_WARNING "%s: Invalid tx_max_coalesce_frames parameter (%d), using %d\n",
				pUmDevice->dev->name,
				tx_max_coalesce_frames[index],
				MAX_TX_MAX_COALESCED_FRAMES);

			tx_max_coalesce_frames[index] = MAX_TX_MAX_COALESCED_FRAMES;
		}
		pDevice->TxMaxCoalescedFrames = tx_max_coalesce_frames[index];
		pUmDevice->tx_curr_coalesce_frames =
			pDevice->TxMaxCoalescedFrames;

		if (stats_coalesce_ticks[index] > MAX_STATS_COALESCING_TICKS) {
			printk(KERN_WARNING "%s: Invalid stats_coalesce_ticks parameter (%d), using %d\n",
				pUmDevice->dev->name,
				stats_coalesce_ticks[index],
				MAX_STATS_COALESCING_TICKS);

			stats_coalesce_ticks[index] =
				MAX_STATS_COALESCING_TICKS;
		}
		pDevice->StatsCoalescingTicks = stats_coalesce_ticks[index];
	}
	else {
		pUmDevice->rx_curr_coalesce_frames =
			DEFAULT_RX_MAX_COALESCED_FRAMES;
		pUmDevice->rx_curr_coalesce_ticks =
			DEFAULT_RX_COALESCING_TICKS;
		pUmDevice->tx_curr_coalesce_frames =
			DEFAULT_TX_MAX_COALESCED_FRAMES;
	}

	if (enable_wol[index]) {
		pDevice->WakeUpModeCap = LM_WAKE_UP_MODE_MAGIC_PACKET;
		pDevice->WakeUpMode = LM_WAKE_UP_MODE_MAGIC_PACKET;
	}
	if (pDevice->EnablePciXFix)
		pDevice->NicSendBd = FALSE;
	else
		pDevice->NicSendBd = TRUE;
#if INCLUDE_TBI_SUPPORT
	pDevice->PollTbiLink = TRUE;
#endif
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_IndicateRxPackets(PLM_DEVICE_BLOCK pDevice)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
	PLM_PACKET pPacket;
	PUM_PACKET pUmPacket;
	struct sk_buff *skb;
	int size;

	while (1) {
		pPacket = (PLM_PACKET)
			QQ_PopHead(&pDevice->RxPacketReceivedQ.Container);
		if (pPacket == 0)
			break;
		pUmPacket = (PUM_PACKET) pPacket;
#if ! defined(NO_PCI_UNMAP)
		pci_unmap_single(pUmDevice->pdev,
				pci_unmap_addr(pUmPacket, map[0]),
				pPacket->u.Rx.RxBufferSize,
				PCI_DMA_FROMDEVICE);
#endif
		if ((pPacket->PacketStatus != LM_STATUS_SUCCESS) ||
			((size = pPacket->PacketSize) > pDevice->RxMtu)) {

			/* reuse skb */
#ifdef TASKLET
			QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
#else
			QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
#endif
			pUmDevice->rx_misc_errors++;
			continue;
		}
		skb = pUmPacket->skbuff;
		skb_put(skb, size);
		skb->pkt_type = 0;
		skb->protocol = eth_type_trans(skb, skb->dev);
		if ((pPacket->Flags & RCV_BD_FLAG_TCP_UDP_CHKSUM_FIELD) &&
			(pDevice->TaskToOffload &
				LM_TASK_OFFLOAD_RX_TCP_CHECKSUM)) {
			if (pPacket->u.Rx.TcpUdpChecksum == 0xffff) {

				skb->ip_summed = CHECKSUM_UNNECESSARY;
#if TIGON3_DEBUG
				pUmDevice->rx_good_chksum_count++;
#endif
			}
			else {
				skb->ip_summed = CHECKSUM_NONE;
				pUmDevice->rx_bad_chksum_count++;
			}
		}
		else {
			skb->ip_summed = CHECKSUM_NONE;
		}
#ifdef NICE_SUPPORT
		if( pUmDevice->nice_rx ) {
			vlan_tag_t *vlan_tag;

			vlan_tag = (vlan_tag_t *) &skb->cb[0];
			if (pPacket->Flags & RCV_BD_FLAG_VLAN_TAG) {
				vlan_tag->signature = 0x7777;
				vlan_tag->tag = pPacket->VlanTag;
			}
			else {
				vlan_tag->signature = 0;
			}
			pUmDevice->nice_rx(skb, pUmDevice->nice_ctx);
		}
		else {
			netif_rx(skb);
		}
#else
		netif_rx(skb);
#endif

#ifdef TASKLET
		pUmPacket->skbuff = 0;
		QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
#else
		skb = dev_alloc_skb(pPacket->u.Rx.RxBufferSize + 2);
		if (skb == 0) {
			pUmPacket->skbuff = 0;
			QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
		}
		else {
			pUmPacket->skbuff = skb;
			skb->dev = pUmDevice->dev;
			skb_reserve(skb, pUmDevice->rx_buf_align);
			QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
		}
#endif
	}
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_CoalesceTxBuffer(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
	PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
	struct sk_buff *skb = pUmPacket->skbuff;
	struct sk_buff *nskb;
#if ! defined(NO_PCI_UNMAP)
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;

	pci_unmap_single(pUmDevice->pdev,
			pci_unmap_addr(pUmPacket, map[0]),
			pci_unmap_len(pUmPacket, map_len[0]),
			PCI_DMA_TODEVICE);
#if MAX_SKB_FRAGS
	{
		int i;

		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			pci_unmap_page(pUmDevice->pdev,
				pci_unmap_addr(pUmPacket, map[i + 1]),
				pci_unmap_len(pUmPacket, map_len[i + 1]),
				PCI_DMA_TODEVICE);
		}
	}
#endif
#endif
	if ((nskb = skb_copy(skb, GFP_ATOMIC))) {
		pUmPacket->lm_packet.u.Tx.FragCount = 1;
		dev_kfree_skb(skb);
		pUmPacket->skbuff = nskb;
		return LM_STATUS_SUCCESS;
	}
	dev_kfree_skb(skb);
	pUmPacket->skbuff = 0;
	return LM_STATUS_FAILURE;
}

/* Returns 1 if not all buffers are allocated */
STATIC int
replenish_rx_buffers(PUM_DEVICE_BLOCK pUmDevice)
{
	PLM_PACKET pPacket;
	PUM_PACKET pUmPacket;
	PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
	struct sk_buff *skb;
	int queue_rx = 0;
	int ret = 0;

	while ((pUmPacket = (PUM_PACKET)
		QQ_PopHead(&pUmDevice->rx_out_of_buf_q.Container)) != 0) {
		pPacket = (PLM_PACKET) pUmPacket;
		if (pUmPacket->skbuff) {
			/* reuse an old skb */
			QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
			queue_rx = 1;
			continue;
		}
		if ((skb = dev_alloc_skb(pPacket->u.Rx.RxBufferSize + 2)) == 0) {
			QQ_PushHead(&pUmDevice->rx_out_of_buf_q.Container,
				pPacket);
			ret = 1;
			break;
		}
		pUmPacket->skbuff = skb;
		skb->dev = pUmDevice->dev;
		skb_reserve(skb, pUmDevice->rx_buf_align);
		QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
		queue_rx = 1;
	}
	if (queue_rx) {
		LM_QueueRxPackets(pDevice);
	}
	return ret;
}

LM_STATUS
MM_IndicateTxPackets(PLM_DEVICE_BLOCK pDevice)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
	PLM_PACKET pPacket;
	PUM_PACKET pUmPacket;
	struct sk_buff *skb;
#if ! defined(NO_PCI_UNMAP) && MAX_SKB_FRAGS
	int i;
#endif

	while (1) {
		pPacket = (PLM_PACKET)
			QQ_PopHead(&pDevice->TxPacketXmittedQ.Container);
		if (pPacket == 0)
			break;
		pUmPacket = (PUM_PACKET) pPacket;
		skb = pUmPacket->skbuff;
#if ! defined(NO_PCI_UNMAP)
		pci_unmap_single(pUmDevice->pdev,
				pci_unmap_addr(pUmPacket, map[0]),
				pci_unmap_len(pUmPacket, map_len[0]),
				PCI_DMA_TODEVICE);
#if MAX_SKB_FRAGS
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			pci_unmap_page(pUmDevice->pdev,
				pci_unmap_addr(pUmPacket, map[i + 1]),
				pci_unmap_len(pUmPacket, map_len[i + 1]),
				PCI_DMA_TODEVICE);
		}
#endif
#endif
		dev_kfree_skb_irq(skb);
		pUmPacket->skbuff = 0;
		QQ_PushTail(&pDevice->TxPacketFreeQ.Container, pPacket);
	}
	if (pUmDevice->tx_full) {
		if (QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container) >=
			(pDevice->TxPacketDescCnt >> 1)) {

			pUmDevice->tx_full = 0;
			netif_wake_queue(pUmDevice->dev);
		}
	}
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_IndicateStatus(PLM_DEVICE_BLOCK pDevice, LM_STATUS Status)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
	struct net_device *dev = pUmDevice->dev;
	LM_FLOW_CONTROL flow_control;

	if (!pUmDevice->opened)
		return LM_STATUS_SUCCESS;

	if (pUmDevice->delayed_link_ind > 0) {
		pUmDevice->delayed_link_ind = 0;
		if (Status == LM_STATUS_LINK_DOWN) {
			printk(KERN_ERR "%s: %s NIC Link is DOWN\n", bcm5700_driver, dev->name);
		}
		else if (Status == LM_STATUS_LINK_ACTIVE) {
			printk(KERN_INFO "%s: %s NIC Link is UP, ", bcm5700_driver, dev->name);
		}
	}
	else {
		if (Status == LM_STATUS_LINK_DOWN) {
			pUmDevice->line_speed = 0;
			printk(KERN_ERR "%s: %s NIC Link is Down\n", bcm5700_driver, dev->name);
		}
		else if (Status == LM_STATUS_LINK_ACTIVE) {
			printk(KERN_INFO "%s: %s NIC Link is Up, ", bcm5700_driver, dev->name);
		}
	}

	if (Status == LM_STATUS_LINK_ACTIVE) {
		if (pDevice->LineSpeed == LM_LINE_SPEED_1000MBPS)
			pUmDevice->line_speed = 1000;
		else if (pDevice->LineSpeed == LM_LINE_SPEED_100MBPS)
			pUmDevice->line_speed = 100;
		else if (pDevice->LineSpeed == LM_LINE_SPEED_10MBPS)
			pUmDevice->line_speed = 10;

		printk("%d Mbps ", pUmDevice->line_speed);

		if (pDevice->DuplexMode == LM_DUPLEX_MODE_FULL)
			printk("full duplex");
		else
			printk("half duplex");

		flow_control = pDevice->FlowControl &
			(LM_FLOW_CONTROL_RECEIVE_PAUSE |
			LM_FLOW_CONTROL_TRANSMIT_PAUSE);
		if (flow_control) {
			if (flow_control & LM_FLOW_CONTROL_RECEIVE_PAUSE) {
				printk(", receive ");
				if (flow_control & LM_FLOW_CONTROL_TRANSMIT_PAUSE)
					printk("& transmit ");
			}
			else {
				printk(", transmit ");
			}
			printk("flow control ON");
		}
		printk("\n");
	}
	return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_FreeRxBuffer(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
	PUM_PACKET pUmPacket;
	struct sk_buff *skb;

	if (pPacket == 0)
		return LM_STATUS_SUCCESS;
	pUmPacket = (PUM_PACKET) pPacket;
	if ((skb = pUmPacket->skbuff))
		dev_kfree_skb(skb);
	pUmPacket->skbuff = 0;
	return LM_STATUS_SUCCESS;
}


